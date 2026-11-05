#include "agent_rules.h"
#include "conditions.h"
#include "rule.h"
#include "rule_engine.h"
#include <memory>
#include "cw2.h"

namespace nano_edr {

std::unique_ptr<MatchRule> CreateScriptHostFromTemp() {
    auto rule = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto images = std::make_unique<AnyOf>();
    images->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    images->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    rule->AddCondition(std::move(images));

    auto temp = std::make_unique<AnyOf>();
    temp->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    rule->AddCondition(std::move(temp));

    rule->AddAction("kill_process");
    return rule;
}

std::unique_ptr<MatchRule> CreateLolbinDownload() {
    auto rule = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto images = std::make_unique<AnyOf>();
    images->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
    images->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
    rule->AddCondition(std::move(images));

    auto cmd = std::make_unique<AnyOf>();
    cmd->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "http:"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "https:"));
    rule->AddCondition(std::move(cmd));

    rule->AddAction("kill_process");
    return rule;
}

std::unique_ptr<MatchRule> CreateHiddenPowershell() {
    auto rule = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto images = std::make_unique<AnyOf>();
    images->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    images->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    rule->AddCondition(std::move(images));

    auto cmd = std::make_unique<AnyOf>();
    cmd->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
    rule->AddCondition(std::move(cmd));

    rule->AddAction("kill_process");
    return rule;
}

std::unique_ptr<MatchRule> CreateAutoStartWrite() {
    auto rule = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kFileCreate,
                               EventType::kFileWrite,
                               EventType::kFileMove}));

    auto paths = std::make_unique<AnyOf>();

    auto path_cond = std::make_unique<AnyOf>();
    path_cond->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
    path_cond->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
    paths->Add(std::move(path_cond));

    auto to_cond = std::make_unique<AnyOf>();
    to_cond->Add(std::make_unique<FieldContains>("to", "\\start menu\\programs\\startup\\"));
    to_cond->Add(std::make_unique<FieldContains>("to", "/start menu/programs/startup/"));
    paths->Add(std::move(to_cond));

    rule->AddCondition(std::move(paths));

    rule->AddAction("quarantine_file");
    return rule;
}

std::unique_ptr<MatchRule> CreateRansomExtension() {
    auto rule = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kFileCreate,
                               EventType::kFileWrite,
                               EventType::kFileMove}));

    auto paths = std::make_unique<AnyOf>();
    paths->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
    paths->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
    rule->AddCondition(std::move(paths));

    rule->AddAction("quarantine_file");
    return rule;
}
std::unique_ptr<SequenceRule> CreateBeacon() {
    auto beacon = std::make_unique<SequenceRule>("script_host_beacon",Severity::kCritical, 10000);
    auto start = std::make_unique<AllOf>();
    start->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    auto images = std::make_unique<AnyOf>();
    images->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    images->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    start->Add(std::move(images));
    auto temp = std::make_unique<AnyOf>();
    temp->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    temp->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    start->Add(std::move(temp));
    beacon->SetFirst(std::move(start));
    beacon->SetSecond(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kNetConnect}));
    beacon->AddAction("kill_process");
    return beacon;
}
RuleEngine BuildAgentRules() {
    RuleEngine engine;
    engine.AddRule(CreateScriptHostFromTemp());
    engine.AddRule(CreateLolbinDownload());
    engine.AddRule(CreateHiddenPowershell());
    engine.AddRule(CreateAutoStartWrite());
    engine.AddRule(CreateRansomExtension());
    engine.AddRule(CreateBeacon());
    return engine;
}

}