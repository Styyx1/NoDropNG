#pragma once

namespace Constants
{
    inline constexpr std::string_view toml_path_default = "Data/SKSE/Plugins/drop-chances.toml";
    inline constexpr std::string_view toml_path_custom = "Data/SKSE/Plugins/drop-chances_custom.toml";
    inline constexpr std::string_view json_path_default = "Data/SKSE/Plugins/drop-chances.json";
    inline constexpr std::string_view json_path_custom = "Data/SKSE/Plugins/drop-chances_custom.json";
}

namespace FormLoader {

	namespace LoaderConstants {
		inline constexpr std::string_view plugin_name = "dropchances.esl";
		static inline RE::FormID rags_chest_ID = 0x1;
		static inline RE::FormID rags_legs_ID = 0x2;
		static inline RE::FormID exception_formlist_ID = 0x3;
		static inline RE::FormID exception_formlist_keyword_ID = 0x4;
	}

	struct Forms {

		static inline RE::TESObjectARMO* rags_chest{ nullptr };
		static inline RE::TESObjectARMO* rags_legs{ nullptr };
		static inline RE::BGSListForm* exception_formlist{ nullptr };
		static inline RE::BGSListForm* exception_formlist_keyword{ nullptr };

		static void LoadForms() {
			logs::info("Loading forms...");
			const auto& dataHandler = RE::TESDataHandler::GetSingleton();
			using namespace LoaderConstants;
			rags_chest = dataHandler->LookupForm<RE::TESObjectARMO>(rags_chest_ID, plugin_name);
			rags_legs = dataHandler->LookupForm<RE::TESObjectARMO>(rags_legs_ID, plugin_name);
			exception_formlist = dataHandler->LookupForm<RE::BGSListForm>(exception_formlist_ID, plugin_name);
			exception_formlist_keyword = dataHandler->LookupForm<RE::BGSListForm>(exception_formlist_keyword_ID, plugin_name);

			if (!rags_chest || !rags_legs || !exception_formlist || !exception_formlist_keyword) {
				logs::error("Failed to load one or more forms from {}", plugin_name);
			} else {
				logs::info("Successfully loaded all forms from {}", plugin_name);
			}
		}
	};
}

namespace Config {

    struct Settings : REX::Singleton<Settings> {
		
        static inline REX::TOML::I32 drop_removal_chance_weapons{ "Drops.Settings", "iDontDropWeapChance", 50 };
		static inline REX::TOML::I32 drop_removal_chance_armor{ "Drops.Settings", "iDontDropArmorChance", 50 };
		static inline REX::TOML::I32 drop_removal_chance_jewelry{ "Drops.Settings", "iDontDropJewelryChance", 50 };
		static inline REX::TOML::Bool replace_armors{ "Drops.Settings", "bReplaceArmors", true };
		static inline REX::TOML::Bool load_replacer_json{ "Drops.Settings", "bLoadReplacerJson", true };  
		static inline REX::TOML::Bool log_to_file{ "Logging", "bLogToFile", false };

		void Update() {
			logs::info("Loading settings...");
			const auto toml = REX::TOML::SettingStore::GetSingleton();
			toml->Init(Constants::toml_path_default.data(), Constants::toml_path_custom.data());
			toml->Load();
			logs::info("...Settings loaded");
		}
		void SaveSettings() {
			const auto toml = REX::TOML::SettingStore::GetSingleton();
			toml->Init(Constants::toml_path_default.data(), Constants::toml_path_custom.data());
			toml->Save();
		}
    };
}