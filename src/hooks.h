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
			logs::info("Registering Event");
			const auto script = RE::ScriptEventSourceHolder::GetSingleton();
			script->AddEventSink<RE::TESDeathEvent>(this);
		}
		void ReplaceArmors(RE::Actor* victim);
		EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override;

	};
}
