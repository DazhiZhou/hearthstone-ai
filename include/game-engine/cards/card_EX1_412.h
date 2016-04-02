#pragma once

#include "card-base.h"

namespace GameEngine {
	namespace Cards {

		class Card_EX1_412
		{
		public:
			static constexpr int card_id = CARD_ID_EX1_412;

			// Raging Worgen

			class Aura : public GameEngine::BoardObjects::AuraEnrage
			{
			private:
				void AddEnrageEnchantment(GameEngine::BoardObjects::Minion & aura_owner)
				{
					constexpr int attack_boost = 1;
					constexpr int hp_boost = 0;
					constexpr int buff_stat = 1 << BoardObjects::MinionStat::FLAG_WINDFURY;

					auto enchantment = std::make_unique<BoardObjects::Enchantment_BuffMinion_C<attack_boost, hp_boost, buff_stat, false>>();

					aura_owner.AddEnchantment(std::move(enchantment), &this->enchantments_manager);
				}

			private: // for comparison
				bool EqualsTo(GameEngine::BoardObjects::Aura const& rhs_base) const
				{
					auto rhs = dynamic_cast<decltype(this)>(&rhs_base);
					if (!rhs) return false;

					return true;
				}

				std::size_t GetHash() const
				{
					std::size_t result = std::hash<int>()(card_id);

					return result;
				}
			};

			static void AfterSummoned(GameEngine::BoardObjects::MinionIterator summoned_minion)
			{
				summoned_minion->AddAura(std::make_unique<Aura>());
			}
		};

	} // namespace Cards
} // namespace GameEngine
