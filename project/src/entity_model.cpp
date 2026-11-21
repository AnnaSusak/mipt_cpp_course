#include "entity_model.h"

#include <cstddef>
#include <format>
#include <string>
#include <vector>

#include "fields.h"

namespace nano_edr {
namespace {
std::string FieldOrEmpty(const Event& event, const std::string& key) {
    const std::string* value = FindField(event, key);
    return value != nullptr ? *value : std::string();
}
constexpr std::size_t kMaxDepth = 64;
constexpr uint64_t kSpawnSourceWindowMs = 60000;

char LowerAscii(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}
bool PathMentioned(const std::string& cmdline, const std::string& path) {
    if (path.empty() || path.size() > cmdline.size()) {
        return false;
    }
    for (std::size_t at = 0; at + path.size() <= cmdline.size(); ++at) {
        std::size_t i = 0;
        while (i < path.size() &&
               LowerAscii(cmdline[at + i]) == LowerAscii(path[i])) {
            ++i;
        }
        if (i == path.size()) {
            return true;
        }
    }
    return false;
}
std::string FileName(const std::string& path) {
    const std::size_t at = path.find_last_of("\\/");
    return at == std::string::npos ? path : path.substr(at + 1);
}

bool Contains(const std::vector<std::string>& list, const std::string& value) {
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (list[i] == value) {
            return true;
        }
    }
    return false;
}

}

ProcessRecord* EntityModel::MutableProcess(const std::string& pid) {
    for (std::size_t i = 0; i < processes_.size(); ++i) {
        if (processes_[i].pid == pid) {
            return &processes_[i];
        }
    }
    if (processes_.size() >= kMaxProcesses) {
        const std::size_t drop = kMaxProcesses / 4;
        processes_.erase(
            processes_.begin(),
            processes_.begin() + static_cast<std::ptrdiff_t>(drop));
    }

    ProcessRecord record;
    record.pid = pid;
    processes_.push_back(record);
    return &processes_.back();
}

FileRecord* EntityModel::MutableFile(const std::string& path) {
    for (std::size_t i = 0; i < files_.size(); ++i) {
        if (files_[i].path == path) {
            return &files_[i];
        }
    }

    if (files_.size() >= kMaxFiles) {
        const std::size_t drop = kMaxFiles / 4;
        files_.erase(files_.begin(),
                     files_.begin() + static_cast<std::ptrdiff_t>(drop));
    }

    FileRecord record;
    record.path = path;
    files_.push_back(record);
    return &files_.back();
}

void EntityModel::NoteFileWrite(const Event& event, const std::string& path,
                                bool created) {
    if (path.empty()) {
        return;
    }
    FileRecord* file = MutableFile(path);
    file->writer_pid = event.pid();
    file->last_write = event.ts();
    file->deleted = false;
    if (created || file->creator_pid.empty()) {
        file->creator_pid = event.pid();
    }
}

void EntityModel::Observe(const Event& event) {
    if (!event.pid().empty()) {
        MutableProcess(event.pid());
    }

    switch (event.event_type()) {
        case EventType::kProcessStart: {
            if (event.pid().empty()) {
                return;
            }
            ProcessRecord* process = MutableProcess(event.pid());
            process->ppid = FieldOrEmpty(event, "ppid");
            process->image = FieldOrEmpty(event, "image");
            process->cmdline = FieldOrEmpty(event, "cmdline");
            process->start = event.ts();
            process->alive = true;
            break;
        }
        case EventType::kProcessEnd: {
            if (!event.pid().empty()) {
                MutableProcess(event.pid())->alive = false;
            }
            break;
        }
        case EventType::kFileCreate:
            NoteFileWrite(event, FieldOrEmpty(event, "path"), true);
            break;
        case EventType::kFileWrite:
            NoteFileWrite(event, FieldOrEmpty(event, "path"), false);
            break;
        case EventType::kFileDelete: {
            const std::string path = FieldOrEmpty(event, "path");
            if (!path.empty()) {
                MutableFile(path)->deleted = true;
            }
            break;
        }
        case EventType::kFileMove: {
            const std::string from = FieldOrEmpty(event, "from");
            if (!from.empty()) {
                MutableFile(from)->deleted = true;
            }
            NoteFileWrite(event, FieldOrEmpty(event, "to"), true);
            break;
        }
        case EventType::kNetConnect:
        case EventType::kOther:
            break;
    }
}
const ProcessRecord* EntityModel::FindProcess(const std::string& pid) const {
    for (std::size_t i = 0; i < processes_.size(); ++i) {
        if (processes_[i].pid == pid) {
            return &processes_[i];
        }
    }
    return nullptr;
}

const FileRecord* EntityModel::FindFile(const std::string& path) const {
    for (std::size_t i = 0; i < files_.size(); ++i) {
        if (files_[i].path == path) {
            return &files_[i];
        }
    }
    return nullptr;
}
std::vector<std::string> EntityModel::GetAncestorChain(
    const std::string& pid) const {
    std::vector<std::string> chain;

    std::string current = pid;
    while (!current.empty() && chain.size() < kMaxDepth) {
        const ProcessRecord* process = FindProcess(current);
        if (process == nullptr) {
            break;
        }
        if (Contains(chain, current)) {
            break;
        }
        chain.push_back(current);
        current = process->ppid;
    }

    return chain;
}

std::vector<std::string> EntityModel::GetProcessTree(
    const std::string& pid) const {
    std::vector<std::string> tree;
    if (FindProcess(pid) == nullptr) {
        return tree;
    }
    tree.push_back(pid);
    for (std::size_t i = 0; i < tree.size() && tree.size() < kMaxDepth; ++i) {
        for (std::size_t j = 0; j < processes_.size(); ++j) {
            if (processes_[j].ppid != tree[i]) {
                continue;
            }
            if (!Contains(tree, processes_[j].pid)) {
                tree.push_back(processes_[j].pid);
            }
        }
    }

    return tree;
}

std::vector<std::string> EntityModel::GetFilesCreatedBy(
    const std::string& pid) const {
    std::vector<std::string> paths;
    if (pid.empty()) {
        return paths;
    }
    for (std::size_t i = 0; i < files_.size(); ++i) {
        if (files_[i].creator_pid == pid) {
            paths.push_back(files_[i].path);
        }
    }
    return paths;
}

const FileRecord* EntityModel::FindSpawnSource(const std::string& pid) const {
    const ProcessRecord* process = FindProcess(pid);
    if (process == nullptr || process->cmdline.empty()) {
        return nullptr;
    }
    const FileRecord* best = nullptr;
    for (std::size_t i = 0; i < files_.size(); ++i) {
        const FileRecord& file = files_[i];
        if (file.path.empty() || file.path.size() > process->cmdline.size()) {
            continue;
        }
        if (process->start.ms < file.last_write.ms ||
            process->start.ms - file.last_write.ms > kSpawnSourceWindowMs) {
            continue;
        }
        if (!PathMentioned(process->cmdline, file.path)) {
            continue;
        }
        if (best == nullptr || best->last_write <= file.last_write) {
            best = &file;
        }
    }
    return best;
}

std::string EntityModel::DescribeChain(const std::string& pid) const {
    const std::vector<std::string> chain = GetAncestorChain(pid);
    if (chain.empty()) {
        return std::string();
    }
    std::string text;
    for (std::size_t i = chain.size(); i > 0; --i) {
        if (!text.empty()) {
            text += '>';
        }
        const std::string& current = chain[i - 1];
        const ProcessRecord* process = FindProcess(current);
        text += current;
        if (process != nullptr && !process->image.empty()) {
            // Только имя файла: полный путь к образу в одну строку детекта
            // не влезает и не читается.
            text += std::format("({})", FileName(process->image));
        }
    }
    return text;
}
}
