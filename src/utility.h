#pragma once

namespace Util {

    inline static bool IsQuestItem(const RE::TESObjectREFR* a_ref)
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
}