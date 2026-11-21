#include <print>
#include <string>
#include <exception>
#include "agent.h"
#include "filesources.h"
#include "os_source.h"
#include "edr_handle.h"
#include "alert_sink.h"
#include "field_traits.h"

int main(int argc, char** argv) {
    try {
        bool quiet = false;
        std::string path;
        bool use_file = false;
        std::string file_name="";
        if (argc < 2) {
            std::print(stderr, "использование: nano-edr <журнал.log> [--quiet] [--file] [--out <файл>]\n");
            return 2;
        }

        for (int i = 1; i < argc; i++) {
            std::string cur = argv[i];
            if (cur == "--quiet") {
                quiet = true;
            } else if (cur == "--file") {
                use_file = true;
            } else if (cur == "--out"){
                if (i == argc - 1) {
                    std::print(stderr, "не указано имя файла после out\n");
                    return 2;
                }
                file_name = argv[++i];
            }else if (!cur.empty() && cur[0] == '-') {
                std::print(stderr, "неизвестная опция: {}\n", cur);
                return 2;
            } else if (path.empty()) {
                path = cur;
            } else {
                std::print(stderr, "лишний аргумент: {}\n", cur);
                return 2;
            }
        }

        if (path.empty()) {
            std::print(stderr, "не указан путь к журналу\n");
            return 2;
        }
        nano_edr::EdrHandle edr(path);
        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", text);
            dedup_period_ms = 10000;
        }
        nano_edr::Agent agent(quiet, dedup_period_ms);

        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::ConsoleSink());
        targets.Add(nano_edr::EdrSink(&edr));
        if (!file_name.empty()) {
            targets.Add(nano_edr::FileSink(file_name));
        }
        agent.Subscribe(std::move(targets));

        std::unique_ptr<nano_edr::IEventSource> source;
        if (use_file) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }
        source->Run(&agent);
        if (edr.Flush() != EDR_OK) {
            std::print(stderr, "[WARN] детекты не отправлены\n");
        }
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
}