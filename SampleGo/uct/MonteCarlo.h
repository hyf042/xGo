/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _MONTE_CARLO_H_
#define _MONTE_CARLO_H_

#include <algorithm>
#include "../naiveSimulator.h"

namespace Go
{
	namespace UCT {
		class MonteCarlo : public NaiveSimulator
		{
			friend class TreeNode;
			
			static const int MAX_SIMULATE_STEP = 1000;
		public:
			MonteCarlo() {}
			MonteCarlo(MonteCarlo &other) {
				*this = other;
			}
			MonteCarlo& operator=(MonteCarlo &other) {
				this->board_size = other.board_size;

				memcpy(this->board, other.board, sizeof(short)*MAX_BOARD*MAX_BOARD);
				memcpy(this->next_stone, other.next_stone, sizeof(Point)*MAX_BOARD*MAX_BOARD);
				this->ko = other.ko;

				return *this;
			}

			override std::string get_name() { return "MonteCarlo"; }
			override Point generate_move(int color) {
				MonteCarlo simulator;
				
				std::vector<Point> moves = generate_good_moves(color);
				Point best(-1, -1);
				float best_value = -1;

				log_format("Move Point: %d\n", moves.size());

				for (auto move : moves) {
					simulator = *this;
					simulator.play_move(move, color);
					float value = 1.f - simulator.simulate(other_color(color));
					log_format("(%d, %d) -> %f", move.r, move.c, value);
					if (value > best_value)
						best_value = value, best = move;
				}

				return best;
			}

			// generate moves limited to max_size
			override std::vector<Point> generate_good_moves(int color, int max_size = 20) {
				typedef std::pair<int, Point> valid_move_pair;
				std::vector<Point> legal_moves = generate_legal_moves(color);
				std::vector<valid_move_pair> optional_moves;

				for (auto move : legal_moves) {
					// evaluate point
					int score = 0;
					int min_dec_liberty = board_size*board_size;
					int max_add_liberty = 0;

					for (int k = 0; k < 4; k++) {
						Point a = move+delta[k];
						if (on_board(a)
							&& get_board(a) == other_color(color))
							min_dec_liberty = std::min(min_dec_liberty, count_liberty(a));
					}

					for (int k = 0; k < 4; k++) {
						Point a = move+delta[k];
						if (on_board(a)
							&& get_board(a) == color) {
							int origin_liberty = count_liberty(a);
							if (origin_liberty >= 4)
								continue;
							max_add_liberty = std::max(max_add_liberty, count_additional_liberty(a, move));
						}
					}

					for (int i = -3; i <= 3; i++) {
						int tmp = 3-std::abs(i);
						for (int j = -tmp; j <= tmp; j++) {
							Point a = move + Point(i, j);
							if (on_board(a)) {
								if (get_board(a) == color)
									score += 20;
								else if (get_board(a) == other_color(color))
									score += 10;
							}
						}
					}

					if (min_dec_liberty < board_size * board_size) {
						if (min_dec_liberty < 4)
							score += (4 - min_dec_liberty) * 100;
					}
					if (max_add_liberty > 0)
						score += max_add_liberty * 100;

					if (score > 0)
						optional_moves.push_back(valid_move_pair(score, move));
				}
				
				std::sort(optional_moves.begin(), optional_moves.end(), [](valid_move_pair a, valid_move_pair b) {
					return a.first > b.first;
				});

				int cnt = 0;
				std::vector<Point> moves;
				for(auto move : optional_moves)
					moves.push_back(move.second);

				if (moves.size() == 0)
					return legal_moves;
				return moves;
			}
			// generate all moves that attach to other point with distance 2
			/* override std::vector<Point> generate_good_moves(int color, int max_size = 20) {
				typedef std::pair<int, Point> valid_move_pair;
				std::vector<Point> legal_moves = generate_legal_moves(color);
				std::vector<Point> moves;

				for (auto move : legal_moves) {
					// evaluate point
					bool flag = false;

					for (int i = -2; i <= 2; i++) {
						int tmp = 2-std::abs(i);
						for (int j = -tmp; j <= tmp; j++) {
							Point a = move + Point(i, j);
							if (on_board(a) && (get_board(a) == color || get_board(a) == other_color(color))) {
								flag = true;
							}
						}
					}
					if (flag)
						moves.push_back(move);
				}

				if (moves.empty())
					return legal_moves;
				else
					return moves;
			}*/
			// generate all the moves that can reduce enemy's liberty or rescue our liberty
			/*override std::vector<Point> generate_good_moves(int color, int max_size = 20) {
				typedef std::pair<int, Point> valid_move_pair;
				std::vector<Point> legal_moves = generate_legal_moves(color);
				std::vector<Point> moves, optional;

				int min_liberty = board_size*board_size;
				for (auto move : legal_moves) {
					
					for (int k = 0; k < 4; k++) {
						Point a = move+delta[k];
						if (on_board(a)
							&& get_board(a) == other_color(color)) {
							int liberty = count_liberty(a);
							if (liberty < min_liberty) {
								min_liberty = liberty;
								optional.clear(), optional.push_back(move);
							}
							else if (liberty == min_liberty)
								optional.push_back(move);
						}
					}
					for (int k = 0; k < 4; k++) {
						Point a = move+delta[k];
						if (on_board(a)
							&& get_board(a) == color) {
							if (count_liberty(a) <= 1)
								if (count_additional_liberty(a, move) > 0)
									moves.push_back(move);
						}
					}
				}

				for (Point sub : optional)
						moves.push_back(sub);

				if (moves.empty())
					return legal_moves;
				else
					return moves;
			}*/
		public:
			float simulate(int color) {
				int now_color = color;
				bool isPass = false;
				for (int i = 0; i < MAX_SIMULATE_STEP; i++) {
					std::vector<Point> moves = generate_legal_moves(now_color);
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
				float tmp = float(cnt)/total;
				if (tmp < 0.2 || 1-tmp < 0.2)
					tmp = tmp+1-1;
				return float(cnt)/total;
			}
		};
	}
}

#endif