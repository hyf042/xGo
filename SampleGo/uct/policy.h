/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _UCT_POLICY_
#define _UCT_POLICY_

#include <algorithm>
#include "../naiveSimulator.h"

namespace Go
{
	namespace UCT {
		class Policy {
		public:
			std::vector<Point> generate_good_moves(NaiveSimulator &board, int color) {
				typedef std::pair<int, Point> valid_move_pair;
				std::vector<Point> legal_moves = board.generate_legal_moves(color);
				std::vector<Point> moves;

				for (auto move : legal_moves) {
					// evaluate point
					bool flag = false;

					for (int i = -2; i <= 2; i++) {
						int tmp = 2-std::abs(i);
						for (int j = -tmp; j <= tmp; j++) {
							Point a = move + Point(i, j);
							if (board.on_board(a) && (board.get_board(a) == color || board.get_board(a) == board.other_color(color))) {
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
			}
				// generate moves limited to max_size
			/*override std::vector<Point> generate_good_moves(int color) {
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
			}*/
			// generate all moves that attach to other point with distance 2
			// generate all the moves that can reduce enemy's liberty or rescue our liberty
			/*override std::vector<Point> generate_good_moves(int color) {
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
			std::vector<Point> next_expand(NaiveSimulator &board, int color) {
				return board.generate_legal_moves(color);
			}
			Point next_simulate(NaiveSimulator &board, int color) {
				 std::vector<Point>& moves = board.generate_legal_moves(color);
				 
				 if (moves.empty())
					 return Point(-1, -1);
				 else
					return moves[rand()%moves.size()];
			}
			float evaluate(NaiveSimulator &board, int color) {
				int cnt = 0, total = 0;
				board.compute_final_status();

				for (int i = 0; i < board.get_board_size(); i++)
					for (int j = 0; j < board.get_board_size(); j++) {
						if (board.is_final_territory(Point(i, j), color))
							cnt++;
						if (board.final_status[i][j] != UNKNOWN)
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