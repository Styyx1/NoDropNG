#pragma once
#include <unordered_set>

namespace Hooks {
	void Install();

	using EventResult = RE::BSEventNotifyControl;

	struct DropData : REX::Singleton<DropData> {
		std::unordered_map<RE::Actor*, std::unordered_set<RE::TESBoundObject*>> itemsToRemove;
	};

	struct OnDeathDropChance : REX::Singleton<OnDeathDropChance>, RE::BSTEventSink<RE::TESDeathEvent> {

		void Register()
		{
			const auto script = RE::ScriptEventSourceHolder::GetSingleton();
			script->AddEventSink<RE::TESDeathEvent>(this);
			logs::info("Registered {}"sv, typeid(RE::TESDeathEvent).name());
		}
		void ReplaceArmors(RE::Actor* victim);
		EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override;

	};

	//struct ContainerMenuLootPrevent : public REX::Singleton<ContainerMenuLootPrevent> {

	//	void InstallHook();

	//	static RE::UI_MESSAGE_RESULTS ProcessMenu(RE::ContainerMenu* a_this, RE::UIMessage& a_message);
	//	static inline REL::Relocation<decltype(&ProcessMenu)> func;

	//	static inline void InvalidateListData(RE::GFxMovieView* a_list, const char* a_method, RE::FxResponseArgs<0>& a_responseArgs)
	//	{
	//		using func_t = decltype(&InvalidateListData);
	//		static REL::Relocation<func_t> func{ REL::ID(82640) };
	//		return func(a_list, a_method, a_responseArgs);
	//	};

	//};
}
