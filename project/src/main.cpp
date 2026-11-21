#include <print>
#include <string>
#include <exception>
#include "agent.h"
#include "filesources.h"
#include "os_source.h"

int main(int argc, char** argv) {
    try {
        bool quiet = false;
        std::string path;
        bool use_file = false;

        if (argc < 2) {
            std::print(stderr, "использование: nano-edr <журнал.log>\n");
            return 2;
        }

        for (int i = 1; i < argc; i++) {
            std::string cur = argv[i];
            if (cur == "--quiet") {
                quiet = true;
            } else if (cur == "--file") {
                use_file = true;
            } else if (!cur.empty() && cur[0] == '-') {
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

        nano_edr::Agent agent(quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (use_file) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }
        source->Run(&agent);
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
}