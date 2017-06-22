#pragma once

#include <utility>
#include "MCTS/selection/Selection.h"
#include "MCTS/simulation/Simulation.h"
#include "MCTS/detail/EpisodeState.h"
#include "MCTS/Statistic.h"
#include "MCTS/ActionType.h"
#include "MCTS/MCTSUpdater.h"
#include "MCTS/board/BoardView.h"
#include "MCTS/board/ActionParameterGetter.h"
#include "MCTS/board/RandomGenerator.h"

namespace mcts
{
	// Traverse and build a game tree in a monte-carlo fashion
	class MCTS
	{
	public:
		typedef selection::TreeNode TreeNode;

		MCTS(Statistic<> & statistic) : action_parameter_getter_(*this), random_generator_(*this),
			statistic_(statistic)
		{
		}

		void Start(TreeNode & root, Stage stage = kStageSelection);

		std::pair<Stage, Result> PerformOneAction(board::Board & board, MCTSUpdater & updater);

	public: // for callbacks: action-parameter-getter and random-generator
		int ChooseAction(ActionType action_type, int choices);

	private:
		int ActionCallback(ActionType action_type, int choices);

		void SwitchToSimulationModeInNextMainAction() {
			// We use a flag here, since we cannot switch to simulation mode in sub-actions.
			flag_switch_to_simulation_ = true;
		}

	private:
		board::ActionParameterGetter action_parameter_getter_;
		board::RandomGenerator random_generator_;

		detail::EpisodeState episode_state_;
		Statistic<> & statistic_;

		selection::Selection selection_stage_;
		simulation::Simulation simulation_stage_;

		bool flag_switch_to_simulation_;
	};
}