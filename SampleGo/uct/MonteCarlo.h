/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _MONTE_CARLO_H_
#define _MONTE_CARLO_H_

#include "../naiveSimulator.h"

namespace Go
{
	namespace UCT {
		class MonteCarlo : public NaiveSimulator
		{
			friend class TreeNode;
			static const int MAX_SIMULATE_STEP = 1000;
		public:
			override std::string get_name() { return "MonteCarlo"; }
			override Point generate_move(int color) {
				MonteCarlo simulator;
				
				std::vector<Point> moves = generate_good_moves(color);
				Point best(-1, -1);
				float best_value = -1;

				log_format("Move Point: %d\n", moves.size());

				for (auto move : moves) {
					simulator.set_board(board_size, board, next_stone);
					simulator.play_move(move, color);
					float value = 1.f - simulator.simulate(other_color(color));
					log_format("(%d, %d) -> %f", move.r, move.c, value);
					if (value > best_value)
						best_value = value, best = move;
				}

				return best;
			}

		public:
			void set_board(int board_size, short board[MAX_BOARD][MAX_BOARD], Point nextStone[MAX_BOARD][MAX_BOARD]) {
				this->board_size = board_size;
				init();

				memcpy(this->board, board, sizeof(short)*MAX_BOARD*MAX_BOARD);
				memcpy(this->next_stone, nextStone, sizeof(Point)*MAX_BOARD*MAX_BOARD);
			}
			void set_board(int board_size, short board[MAX_BOARD][MAX_BOARD]) {
				this->board_size = board_size;
				init();

				memcpy(this->board, board, sizeof(short)*MAX_BOARD*MAX_BOARD);
				reconstruct_next_stone();
			}

			float simulate(int color) {
				int now_color = color;
				bool isPass = false;
				for (int i = 0; i < MAX_SIMULATE_STEP; i++) {
					std::vector<Point> moves = generate_good_moves(now_color);
					if (moves.empty()) {
						if (isPass)
							break;
						else
							isPass = true;
					}
					else {
						isPass = false;
						play_move(moves[rand()%moves.size()], now_color);
					}
					now_color = other_color(now_color);
				}

				int cnt = 0, total = 0;
				compute_final_status();
				
				for (int i = 0; i < board_size; i++)
					for (int j = 0; j < board_size; j++) {
						if (is_final_territory(Point(i, j), color))
							cnt++;
						if (final_status[i][j] != UNKNOWN)
							total++;
					}
				if (total == 0)
					return 0.5f;
				return float(cnt)/total;
			}
		};
	}
}

#endif