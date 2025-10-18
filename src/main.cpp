#include "hooks.h"
#include "config.h"
#include "utility.h"

void InitListener(SKSE::MessagingInterface::Message* a_msg) {

	switch (a_msg->type) {
	case SKSE::MessagingInterface::kInputLoaded:

		break;
	case SKSE::MessagingInterface::kDataLoaded:
		FormLoader::Forms::LoadForms();
		Hooks::OnDeathDropChance::GetSingleton()->Register();
		Util::LoaderUtil::JSONLoader::LoadReplacements();
		
		break;

	case SKSE::MessagingInterface::kPostLoadGame:

		break;

	case SKSE::MessagingInterface::kNewGame:

		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	Config::Settings::GetSingleton()->Update();
	SKSE::GetMessagingInterface()->RegisterListener(InitListener);

	return true;
}
