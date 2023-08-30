#include "Distributor.h"
#include "Hooks.h"
#include "Logging.h"
#include "Settings.h"

#include "SKSE/Interfaces.h"

void Listener(SKSE::MessagingInterface::Message* message) noexcept
{
    if (message->type <=> SKSE::MessagingInterface::kPostLoad == 0)
    {
        if (!GetModuleHandle(L"po3_Tweaks"))
        {
            logger::info("po3_Tweaks.dll not found, installing EditorID caching fix...");
            Hooks::Install();
        }
    }
    if (message->type <=> SKSE::MessagingInterface::kDataLoaded == 0)
    {
        Settings::LoadSettings();
        Distributor::CheckConflicts();
        Distributor::PrepareDistribution();
        Distributor::Distribute();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    InitializeLogging();

    const auto plugin{ SKSE::PluginDeclaration::GetSingleton() };
    const auto version{ plugin->GetVersion() };

    logger::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);

    if (const auto messaging{ SKSE::GetMessagingInterface() }; !messaging->RegisterListener(Listener))
        return false;

    logger::info("{} has finished loading.", plugin->GetName());

    return true;
}
