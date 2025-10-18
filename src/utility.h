#pragma once

#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace UtilConstants {
	inline std::filesystem::path replacementFile = "Data/SKSE/Plugins/DropReplacer.json";
}

namespace Util {

    inline bool ActorHasQuestObjectEquipped(RE::Actor* actor) {
        if (actor) {
            auto* rightHandItem = actor->GetEquippedEntryData(false);
            if (rightHandItem) {
                if (rightHandItem->IsQuestObject()) {
                    return true;
                }
            }

            auto* leftHandItem = actor->GetEquippedEntryData(true);
            if (leftHandItem) {
                if (leftHandItem->IsQuestObject()) {
                    return true;
                }
            }
        }

        return false;
    }

    inline bool IsQuestItem(const RE::TESObjectREFR* a_ref)
    {
        if (const auto xAliases = a_ref->extraList.GetByType<RE::ExtraAliasInstanceArray>(); xAliases) {
            RE::BSReadLockGuard locker(xAliases->lock);

            return std::ranges::any_of(xAliases->aliases, [](const auto& aliasData) {
                const auto alias = aliasData ? aliasData->alias : nullptr;
                return alias && alias->IsQuestObject();
                });
        }

        return a_ref->HasQuestObject();
    }

	struct RandomGenerator {
        static std::mt19937& GetRNG()
        {
            static std::mt19937 gen(std::random_device{}());
            return gen;
        }

        static int GetRandomInt(int a_min, int a_max)
        {
            std::uniform_int_distribution<int> distrib(a_min, a_max);
            return distrib(GetRNG());
        }

        static float GetRandomFloat(float a_min, float a_max)
        {
            std::uniform_real_distribution<float> distrib(a_min, a_max);
            return distrib(GetRNG());
        }
	};



    namespace LoaderUtil {

        

        struct FormUtils {

            // Convert a string like "Skyrim.esm|0x12345" -> TESForm*
            static inline RE::TESForm* get_form_from_string(const std::string& formIDstring)
            {
                std::istringstream ss{ formIDstring };
                std::string plugin, id;

                std::getline(ss, plugin, '|');
                std::getline(ss, id);

                if (plugin.empty() || id.empty()) {
                    logs::warn("Invalid form string: '{}'", formIDstring);
                    return nullptr;
                }

                RE::FormID rawFormID{};
                std::istringstream(id) >> std::hex >> rawFormID;

                if (!rawFormID) {
                    logs::warn("Invalid formID '{}' in string '{}'", id, formIDstring);
                    return nullptr;
                }

                auto* dataHandler = RE::TESDataHandler::GetSingleton();
                if (!dataHandler) {
                    SKSE::stl::report_and_fail(std::format("TESDataHandler not available while parsing '{}'", formIDstring));
                    return nullptr;
                }

                auto* form = dataHandler->LookupForm(rawFormID, plugin);
                if (!form) {
                    logs::warn("Failed to find form '{}'", formIDstring);
                }

                return form;
            }
        };

        struct JSONLoader {

            static inline std::unordered_map<RE::TESForm*, RE::TESForm*> loadedReplacements;

            using json = nlohmann::json;

            // Load a JSON file from disk safely
            static json load_json(const std::filesystem::path& path)
            {
                std::ifstream file(path);
                if (!file.is_open()) {
                    logs::error("Could not open JSON file '{}'", path.string());
                    return {};
                }

                json j;
                try {
                    file >> j;
                }
                catch (const std::exception& e) {
                    logs::error("Failed to parse JSON '{}': {}", path.string(), e.what());
                    return {};
                }
                return j;
            }

            // Load an array of form strings into a container (like std::unordered_set<RE::TESForm*>)
            template <typename Container>
            static void load_forms_from_json(Container& container,
                const std::filesystem::path& path,
                std::string_view key)
            {
                auto j = load_json(path);
                if (!j.contains(key) || !j[key].is_array()) {
                    logs::warn("Missing or invalid key '{}' in '{}'", key, path.string());
                    return;
                }

                const auto& arr = j[key];
                for (const auto& elem : arr) {
                    if (!elem.is_string())
                        continue;

                    const std::string& formStr = elem.get_ref<const std::string&>();
                    RE::TESForm* form = FormUtils::get_form_from_string(formStr);

                    if (form) {
                        container.insert(form);
                        logs::debug("Loaded form '{}'", formStr);
                    }
                    else {
                        logs::warn("Invalid form '{}'", formStr);
                    }
                }

                logs::info("Loaded {} valid forms from '{}'", container.size(), path.string());
            }

            /// Load key-value pairs of forms into a map (e.g. std::unordered_map<RE::TESForm*, RE::TESForm*>)
            template <typename Map>
            static void load_form_pairs_from_json(Map& map,
                const std::filesystem::path& path,
                std::string_view key)
            {
                auto j = load_json(path);
                if (!j.contains(key) || !j[key].is_object()) {
                    logs::warn("Missing or invalid key '{}' in '{}'", key, path.string());
                    return;
                }

                const auto& obj = j[key];
                for (const auto& [srcStr, dstVal] : obj.items()) {
                    if (!dstVal.is_string())
                        continue;

                    RE::TESForm* src = FormUtils::get_form_from_string(srcStr);
                    RE::TESForm* dst = FormUtils::get_form_from_string(dstVal.get<std::string>());

                    if (src && dst) {
                        map[src] = dst;
                        logs::debug("Loaded form pair: '{}' -> '{}'", srcStr, dstVal.get<std::string>());
                    }
                    else {
                        logs::warn("Invalid form pair: '{}' -> '{}'", srcStr, dstVal.get<std::string>());
                    }
                }

                logs::info("Loaded {} form pairs from '{}'", map.size(), path.string());
            }

            static void LoadReplacements()
            {
                loadedReplacements.clear();
                JSONLoader::load_form_pairs_from_json(loadedReplacements, UtilConstants::replacementFile, "replacements");
                logs::info("Loaded {} replacements from '{}'", loadedReplacements.size(), UtilConstants::replacementFile.string());
            }

        };

        

    }
}