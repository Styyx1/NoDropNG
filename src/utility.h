#pragma once

#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace UtilConstants {
	inline std::filesystem::path replacementFolder = "Data/SKSE/Plugins/DropReplacer";
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

            static inline RE::TESForm* get_form_from_string(const std::string& formIDstring)
            {
                if (formIDstring.empty()) {
                    logs::warn("Empty form string");
                    return nullptr;
                }

                if (formIDstring.find('|') != std::string::npos) {
                    std::istringstream ss{ formIDstring };
                    std::string plugin, id;
                    std::getline(ss, plugin, '|');
                    std::getline(ss, id);

                    if (plugin.empty() || id.empty()) {
                        return nullptr;
                    }

                    RE::FormID rawFormID{};
                    std::istringstream(id) >> std::hex >> rawFormID;

                    if (!rawFormID) {
                        return nullptr;
                    }

                    if (auto* dataHandler = RE::TESDataHandler::GetSingleton()) {
                        auto* form = dataHandler->LookupForm(rawFormID, plugin);
                        return form;
                    }
                    return nullptr;
                }

                if (auto* form = RE::TESForm::LookupByEditorID(formIDstring)) {
                    return form;
                }

                logs::warn("Could not find form '{}'", formIDstring);
                return nullptr;
            }
        };

        struct JSONLoader {

            static inline std::unordered_map<RE::TESForm*, RE::TESForm*> loadedReplacements;

            using json = nlohmann::json;

            static json load_json(const std::filesystem::path& path)
            {
                const std::string pathName = path.string();
                std::ifstream file(path);
                if (!file.is_open()) {
                    logs::error("Could not open JSON file '{}'", pathName);
                    return {};
                }

                json j;
                try {
                    file >> j;
                }
                catch (const std::exception& e) {
                    logs::error("Failed to parse JSON '{}': {}", pathName, e.what());
                    return {};
                }
                return j;
            }

            template <typename Container>
            static void load_forms_from_json(Container& container,
                const std::filesystem::path& path,
                std::string_view key)
            {
                auto j = load_json(path);
                if (!j.contains(key) || !j[key].is_array()) {
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

            template <typename Map>
            static void load_form_pairs_from_json(Map& map,
                const std::filesystem::path& path,
                std::string_view key)
            {
                auto j = load_json(path);
                if (!j.contains(key) || !j[key].is_object()) {
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
                    }
                }
                const std::string pathName = path.string();
                logs::info("Loaded {} form pairs from '{}'", map.size(), pathName);
            }

            static void load_form_pairs_from_folder(std::unordered_map<RE::TESForm*, RE::TESForm*>& map,
                const std::filesystem::path& folder,
                std::string_view key)
            {
                const std::string pathName = folder.string();

                if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) {
                    logs::warn("Folder '{}' does not exist or is not a directory.", pathName);
                    return;
                }

                size_t totalCount = 0;

                for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                    if (!entry.is_regular_file())
                        continue;

                    const auto& path = entry.path();
                    if (path.extension() != ".json")
                        continue;

                    logs::info("Loading replacement data from '{}'", pathName);

                    std::unordered_map<RE::TESForm*, RE::TESForm*> temp;
                    JSONLoader::load_form_pairs_from_json(temp, path, key);

                    totalCount += temp.size();

                    // Merge temp into main map (overwriting duplicates)
                    map.insert(temp.begin(), temp.end());
                }

                logs::info("Loaded {} total replacements from folder '{}'", totalCount, pathName);
            }

            static void LoadReplacements()
            {
                loadedReplacements.clear();
                JSONLoader::load_form_pairs_from_folder(loadedReplacements, UtilConstants::replacementFolder, "replacements");
                const std::string path = UtilConstants::replacementFolder.string();
                logs::info("Loaded {} replacements from '{}'", loadedReplacements.size(), path);
            }

        };

        

    }
}