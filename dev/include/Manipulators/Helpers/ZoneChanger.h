#pragma once

#include "Entity/Card.h"
#include "Entity/CardType.h"
#include "Entity/CardZone.h"
#include "EntitiesManager/CardRef.h"
#include "State/State.h"
#include "State/PlayerIdentifier.h"
#include "State/Player.h"
#include "State/Utils/DefaultZonePosPolicy.h"

namespace Manipulators
{
	namespace Helpers
	{
		template <Entity::CardZone ChangingCardZone, Entity::CardType ChangingCardType>
		class ZoneChanger
		{
		public:
			ZoneChanger(EntitiesManager& mgr, CardRef card_ref, Entity::Card &card) : mgr_(mgr), card_ref_(card_ref), card_(card) {}

			template <Entity::CardZone ChangeToZone,
				typename std::enable_if_t<State::Utils::ForcelyUseDefaultZonePos<ChangeToZone, ChangingCardType>::value, nullptr_t> = nullptr>
				void ChangeTo(State::State & state, State::PlayerIdentifier player_identifier)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				int new_pos = State::Utils::DefaultZonePosGetter<ChangeToZone, ChangingCardType>()(player);
				return ChangeToInternal<ChangeToZone>(state, player_identifier, new_pos);
			}

			template <Entity::CardZone ChangeToZone,
				typename std::enable_if_t<!State::Utils::ForcelyUseDefaultZonePos<ChangeToZone, ChangingCardType>::value, nullptr_t> = nullptr>
				void ChangeTo(State::State & state, State::PlayerIdentifier player_identifier, int pos)
			{
				return ChangeToInternal<ChangeToZone>(state, player_identifier, pos);
			}

			void Add(State::State & state)
			{
				switch (card_.GetZone())
				{
				case Entity::kCardZoneDeck:
					return AddToDeckZone(state);
				case Entity::kCardZoneHand:
					return AddToHandZone(state);
				case Entity::kCardZonePlay:
					return AddToPlayZone(state);
				case Entity::kCardZoneGraveyard:
					return AddToGraveyardZone(state);
				}
			}

		private:
			template <Entity::CardZone ChangeToZone>
			void ChangeToInternal(State::State & state, State::PlayerIdentifier player_identifier, int pos)
			{
				Remove(state);

				auto location_setter = card_.GetLocationSetter();
				location_setter.SetPlayerIdentifier(player_identifier);
				location_setter.SetZone(ChangeToZone);
				location_setter.SetZonePosition(pos);

				Add(state);
			}

			void Remove(State::State & state)
			{
				switch (ChangingCardZone)
				{
				case Entity::kCardZoneDeck:
					return RemoveFromDeckZone(state);
				case Entity::kCardZoneHand:
					return RemoveFromHandZone(state);
				case Entity::kCardZonePlay:
					return RemoveFromPlayZone(state);
				case Entity::kCardZoneGraveyard:
					return RemoveFromGraveyardZone(state);
				}
			}

			void AddToDeckZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				player.deck_.GetLocationManipulator().Insert(mgr_, card_ref_);
			}
			void RemoveFromDeckZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				player.deck_.GetLocationManipulator().Remove(mgr_, card_.GetZonePosition());
			}

			void AddToHandZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				player.hand_.GetLocationManipulator().Insert(mgr_, card_ref_);
			}
			void RemoveFromHandZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				player.hand_.GetLocationManipulator().Remove(mgr_, card_.GetZonePosition());
			}

			void AddToPlayZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());

				switch (ChangingCardType)
				{
				case Entity::kCardTypeMinion:
					return player.minions_.GetLocationManipulator().Insert(mgr_, card_ref_);
				case Entity::kCardTypeWeapon:
					return player.weapon_.Equip(card_ref_);
				case Entity::kCardTypeSecret:
					return player.secrets_.Add(card_.GetCardId(), card_ref_);
				}
			}
			void RemoveFromPlayZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());

				switch (ChangingCardType)
				{
				case Entity::kCardTypeMinion:
					return player.minions_.GetLocationManipulator().Remove(mgr_, card_.GetZonePosition());
				case Entity::kCardTypeWeapon:
					return player.weapon_.Destroy();
				case Entity::kCardTypeSecret:
					return player.secrets_.Remove(card_.GetCardId());
				}
			}

			void AddToGraveyardZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				player.graveyard_.GetLocationManipulator<ChangingCardType>().Insert(mgr_, card_ref_);
			}
			void RemoveFromGraveyardZone(State::State & state)
			{
				State::Player & player = state.players.Get(card_.GetPlayerIdentifier());
				player.graveyard_.GetLocationManipulator<ChangingCardType>().Remove(mgr_, card_.GetZonePosition());
			}

		private:
			EntitiesManager & mgr_;
			CardRef card_ref_;
			Entity::Card & card_;
		};

		template <Entity::CardType ChangingCardType>
		class ZoneChangerWithUnknownZone
		{
		public:
			ZoneChangerWithUnknownZone(EntitiesManager& mgr, CardRef card_ref, Entity::Card &card) : mgr_(mgr), card_ref_(card_ref), card_(card) {}

			template <Entity::CardZone ChangeToZone>
			void ChangeTo(State::State & state, State::PlayerIdentifier player_identifier)
			{
				switch (card_.GetZone())
				{
				case Entity::kCardZoneDeck:
					return ZoneChanger<Entity::kCardZoneDeck, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardZoneGraveyard:
					return ZoneChanger<Entity::kCardZoneGraveyard, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardZoneHand:
					return ZoneChanger<Entity::kCardZoneHand, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardZonePlay:
					return ZoneChanger<Entity::kCardZonePlay, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardZonePutASide:
					return ZoneChanger<Entity::kCardZonePutASide, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardZoneRemoved:
					return ZoneChanger<Entity::kCardZoneRemoved, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardZoneSecret:
					return ZoneChanger<Entity::kCardZoneSecret, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				default:
					throw std::exception("Unknown card zone");
				}
			}

			template <Entity::CardZone ChangeToZone>
			void ChangeTo(State::State & state, State::PlayerIdentifier player_identifier, int pos)
			{
				switch (card_.GetZone())
				{
				case Entity::kCardZoneDeck:
					return ZoneChanger<Entity::kCardZoneDeck, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardZoneGraveyard:
					return ZoneChanger<Entity::kCardZoneGraveyard, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardZoneHand:
					return ZoneChanger<Entity::kCardZoneHand, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardZonePlay:
					return ZoneChanger<Entity::kCardZonePlay, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardZonePutASide:
					return ZoneChanger<Entity::kCardZonePutASide, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardZoneRemoved:
					return ZoneChanger<Entity::kCardZoneRemoved, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardZoneSecret:
					return ZoneChanger<Entity::kCardZoneSecret, ChangingCardType>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				default:
					throw std::exception("Unknown card zone");
				}
			}

			void Add(State::State & state)
			{
				switch (card_.GetZone())
				{
				case Entity::kCardZoneDeck:
					return ZoneChanger<Entity::kCardZoneDeck, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardZoneGraveyard:
					return ZoneChanger<Entity::kCardZoneGraveyard, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardZoneHand:
					return ZoneChanger<Entity::kCardZoneHand, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardZonePlay:
					return ZoneChanger<Entity::kCardZonePlay, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardZonePutASide:
					return ZoneChanger<Entity::kCardZonePutASide, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardZoneRemoved:
					return ZoneChanger<Entity::kCardZoneRemoved, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardZoneSecret:
					return ZoneChanger<Entity::kCardZoneSecret, ChangingCardType>(mgr_, card_ref_, card_).Add(state);
				default:
					throw std::exception("Unknown card zone");
				}
			}

		private:
			EntitiesManager & mgr_;
			CardRef card_ref_;
			Entity::Card & card_;
		};

		class ZoneChangerWithUnknownZoneUnknownType
		{
		public:
			ZoneChangerWithUnknownZoneUnknownType(EntitiesManager& mgr, CardRef card_ref, Entity::Card &card) : mgr_(mgr), card_ref_(card_ref), card_(card) {}

			template <Entity::CardZone ChangeToZone>
			void ChangeTo(State::State & state, State::PlayerIdentifier player_identifier)
			{
				switch (card_.GetCardType())
				{
				case Entity::kCardTypeMinion:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeMinion>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardTypeHeroPower:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeHeroPower>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardTypeSecret:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeSecret>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardTypeSpell:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeSpell>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				case Entity::kCardTypeWeapon:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeWeapon>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier);
				default:
					throw std::exception("unknown card type");
				}
			}

			template <Entity::CardZone ChangeToZone>
			void ChangeTo(State::State & state, State::PlayerIdentifier player_identifier, int pos)
			{
				switch (card_.GetCardType())
				{
				case Entity::kCardTypeMinion:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeMinion>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardTypeHeroPower:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeHeroPower>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardTypeSecret:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeSecret>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardTypeSpell:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeSpell>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				case Entity::kCardTypeWeapon:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeWeapon>(mgr_, card_ref_, card_).ChangeTo<ChangeToZone>(state, player_identifier, pos);
				default:
					throw std::exception("unknown card type");
				}
			}

			void Add(State::State & state)
			{
				switch (card_.GetCardType())
				{
				case Entity::kCardTypeMinion:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeMinion>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardTypeHeroPower:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeHeroPower>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardTypeSecret:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeSecret>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardTypeSpell:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeSpell>(mgr_, card_ref_, card_).Add(state);
				case Entity::kCardTypeWeapon:
					return ZoneChangerWithUnknownZone<Entity::kCardTypeWeapon>(mgr_, card_ref_, card_).Add(state);
				default:
					throw std::exception("unknown card type");
				}
			}

		private:
			EntitiesManager & mgr_;
			CardRef card_ref_;
			Entity::Card & card_;
		};
	}
}