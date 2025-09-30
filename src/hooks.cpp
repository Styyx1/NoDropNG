#include "hooks.h"
#include "utility.h"
#include "config.h"
namespace Hooks
{
	void OnDeathDropChance::ReplaceArmors(RE::Actor* victim)
	{
		if (!FormLoader::Forms::rags_chest || !FormLoader::Forms::rags_legs) {
			logs::error("Replacement Armor Forms not loaded!");
			return;
		}
		if (!victim) {
			return;
		}
		auto dropData = DropData::GetSingleton();
		auto it = dropData->itemsToRemove.find(victim);
		if (it == dropData->itemsToRemove.end() || it->second.empty()) {
			return;
		}
		auto& replaceList = it->second;
		for (auto gear : replaceList) {
			if (!gear) {
				continue;
			}
			auto armor = gear->As<RE::TESObjectARMO>();
			if(!armor) {
				continue;
			}
			switch (armor->GetSlotMask())
			{
			case RE::BGSBipedObjectForm::BipedObjectSlot::kBody:
				victim->AddObjectToContainer(FormLoader::Forms::rags_chest, nullptr, 1, nullptr);
				RE::ActorEquipManager::GetSingleton()->EquipObject(victim, FormLoader::Forms::rags_chest, nullptr, 1, armor->GetEquipSlot(), true, true, false, true);
				break;
			case RE::BGSBipedObjectForm::BipedObjectSlot::kFeet:
				victim->AddObjectToContainer(FormLoader::Forms::rags_legs, nullptr, 1, nullptr);
				RE::ActorEquipManager::GetSingleton()->EquipObject(victim, FormLoader::Forms::rags_legs, nullptr, 1, armor->GetEquipSlot(), true, true, false, true);
				break;
			default:
				break;
			}
		}
	}

	EventResult OnDeathDropChance::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		RE::PlayerCharacter *const &player = RE::PlayerCharacter::GetSingleton();

		if (!a_event || !a_event->actorDying || a_event->actorDying.get() == player) {
			return EventResult::kContinue;
		}
		const auto actor = a_event->actorDying->As<RE::Actor>();
		if (!actor || actor->IsPlayerTeammate()) {
			return EventResult::kContinue;
		}
		if (actor->IsEssential() || actor->IsDead(false)) {
			return EventResult::kContinue;
		}
		auto actorInventory = actor->GetInventory();
		if (actorInventory.empty()) {
			return EventResult::kContinue;
		}
		auto dropData = DropData::GetSingleton();
		for (auto& item : actorInventory) {
			if(!item.first || !item.second.first || item.second.first <= 0) {
				continue;
			}
			if(Util::IsQuestItem(item.first->AsReference())) {
				logs::info("Skipping {} because it's a quest item", item.first->GetName());
				continue;
			}
			if (item.second.second.get() && item.second.second.get()->GetEnchantment() != nullptr) {
				logs::info("Skipping {} because it's enchanted", item.first->GetName());
				continue;
			}
			if (FormLoader::Forms::exception_formlist && FormLoader::Forms::exception_formlist->HasForm(item.first)) {
				logs::info("Skipping {} because it's in the exception list", item.first->GetName());
				continue;
			}

			if (FormLoader::Forms::exception_formlist_keyword && item.first->HasKeywordInList(FormLoader::Forms::exception_formlist_keyword, false)) {
				logs::info("Skipping {} because it has an exception keyword", item.first->GetName());
			}

			auto entryData = item.second.second.get();
			switch (item.first->formType.get())	{
			case RE::FormType::Weapon:
				if (entryData->IsWorn()) {
					if (item.first->As<RE::TESObjectWEAP>()->GetPlayable()) {						
						int chance = Config::Settings::drop_removal_chance_armor.GetValue();
						logs::info("Chance to remove {} from {} is {}%", item.first->GetName(), actor->GetName(), chance);
						if (Util::RandomGenerator::GetRandomFloat(0.0f, 100.0f) < static_cast<float>(chance)) {
							actor->RemoveItem(item.first, item.second.first, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);						
							logs::info("Removed {} from {}", item.first->GetName(), actor->GetName());
						}
					}
				}
				break;				
			case RE::FormType::Armor:
				if (entryData->IsWorn()) {
					if (item.first->As<RE::TESObjectARMO>()->GetPlayable()) {
						int chance = Config::Settings::drop_removal_chance_armor.GetValue();
						logs::info("Chance to remove {} from {} is {}%", item.first->GetName(), actor->GetName(), chance);
						if (Util::RandomGenerator::GetRandomFloat(0.0f, 100.0f) < static_cast<float>(chance)) {
							dropData->itemsToRemove[actor].insert(item.first);
							actor->RemoveItem(item.first, item.second.first, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
							logs::info("Removed {} from {}", item.first->GetName(), actor->GetName());
						}
					}
				}
				break;
			default:
				break;
			}
		}
		if (dropData->itemsToRemove.contains(actor) && !dropData->itemsToRemove[actor].empty()) {
			ReplaceArmors(actor);
			dropData->itemsToRemove.erase(actor);
		}
		return EventResult::kContinue;
	}

