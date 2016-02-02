#include <string>
#include <sstream>
#include <stdexcept>

#include "game-engine/move.h"

namespace GameEngine {

std::string Move::GetDebugString() const
{
	std::ostringstream oss;

	switch (this->action)
	{
		case Move::ACTION_GAME_FLOW:
			oss << "[Game flow]";
			break;

		case Move::ACTION_PLAY_HAND_CARD_MINION:
			oss << "[Play hand card minion] hand idx = " << this->data.play_hand_card_minion_data.hand_card;
#ifdef CHOOSE_WHERE_TO_PUT_MINION
			oss << ", put location = " << this->data.play_hand_card_minion_data.location;
#endif
			break;

		case Move::ACTION_OPPONENT_PLAY_MINION:
			oss << "[Opponent play minion] card: " << this->data.opponent_play_minion_data.card.id;
#ifdef CHOOSE_WHERE_TO_PUT_MINION
			oss << ", put location = " << this->data.opponent_play_minion_data.location;
#endif
			break;

		case Move::ACTION_PLAYER_ATTACK:
			oss << "[Player Attack] attacking = " << this->data.player_attack_data.attacker_idx
				<< ", attacked = " << this->data.player_attack_data.attacked_idx;
			break;

		case Move::ACTION_OPPONENT_ATTACK:
			oss << "[Opponent Attack] attacking = " << this->data.opponent_attack_data.attacker_idx
				<< ", attacked = " << this->data.opponent_attack_data.attacked_idx;
			break;

		case Move::ACTION_END_TURN:
			oss << "[End turn]";
			break;

		case Move::ACTION_UNKNOWN:
			throw std::runtime_error("unknown action for Move::DebugPrint()");
			break;
	}

	return oss.str();
}

} // namespace GameEngine
