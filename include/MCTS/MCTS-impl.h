#pragma once

#include <cstdlib>

#include "MCTS/MCTS.h"
#include "MCTS/board/Board-impl.h"
#include "MCTS/board/BoardView-impl.h"
#include "MCTS/board/ActionParameterGetter-impl.h"
#include "MCTS/board/RandomGenerator-impl.h"

namespace mcts
{
	inline void MCTS::Start(TreeNode * root, Stage stage) {
		episode_state_.Start(stage);
		assert(episode_state_.GetStage() == stage);

		if (stage == kStageSelection) {
			new_node_created_ = false;
			selection_stage_.StartEpisode(root);
		}
	}

	// Never returns kResultInvalid
	//    Will automatically retry if an invalid action is applied
	inline MCTS::PerformResult MCTS::PerformOneAction(board::Board & board, MCTSUpdater & updater) {
		episode_state_.StartAction(board);

		// TODO [PROFILING]:
		// We save the board every time here. This is just for the case an invalid action is applied
		// Ideally, invalid actions should be rarely happened.
		// So if copy a board is comparatively expensive, we need to decide if this is benefitial.
		assert(episode_state_.IsValid());
		board::Board const saved_board = episode_state_.GetBoard();

		if (episode_state_.GetStage() == kStageSelection) {
			selection_stage_.StartNewAction();
		}
		else {
			assert(episode_state_.GetStage() == kStageSimulation);
			simulation_stage_.StartNewAction();
		}

		// sometimes an action might in fact an INVALID action
		// here use a loop to retry on those cases
		while (true)
		{
			int choices = episode_state_.GetBoard().GetActionsCount();
			int choice = this->ChooseAction(ActionType(ActionType::kMainAction), choices);

			Result result = Result::kResultInvalid;
			if (episode_state_.IsValid()) {
				result = episode_state_.GetBoard().ApplyAction(choice, random_generator_, action_parameter_getter_);
			}

			if (result == Result::kResultInvalid) {
				if (episode_state_.GetStage() == kStageSelection) {
					selection_stage_.ReportInvalidAction();
					selection_stage_.RestartAction();
				}
				else {
					assert(episode_state_.GetStage() == kStageSimulation);
					simulation_stage_.ReportInvalidAction();
					simulation_stage_.RestartAction();
				}

				episode_state_.GetBoard() = saved_board;
				episode_state_.SetValid();

				statistic_.ApplyActionFailed();
				continue;
			}

			// action applied successfully
			assert(episode_state_.IsValid());
			statistic_.ApplyActionSucceeded();
			updater.PushBackNodes(selection_stage_.GetTraversedPath());

			MCTS::PerformResult perform_result;
			perform_result.new_node_created = new_node_created_;
			perform_result.result = result;
			perform_result.node = selection_stage_.GetCurrentNode();
			return perform_result;
		}
	}

	inline int MCTS::ChooseAction(ActionType action_type, int choices)
	{
		return ActionCallback(action_type, choices);
	}

	inline int MCTS::ActionCallback(ActionType action_type, int choices)
	{
		assert(choices > 0);

		int choice = -1;
		if (episode_state_.GetStage() == kStageSelection) {
			choice = selection_stage_.GetAction(episode_state_.GetBoard(), action_type, choices, &new_node_created_);
		}
		else {
			assert(episode_state_.GetStage() == kStageSimulation);
			choice = simulation_stage_.GetAction(episode_state_.GetBoard(), action_type, choices);
		}

		if (choice < 0) episode_state_.SetInvalid();
		return choice;
	}
}
