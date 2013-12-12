/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _MINMAX_H_
#define _MINMAX_H_

#include <cstdlib>
#include <ctime>
#include "../naiveSimulator.h"

namespace Go
{
	namespace MinMax {
		class BoardData : public NaiveSimulator {
		public:
			BoardData() {}
			BoardData(BoardData &other) {*this = other;}
			BoardData& operator=(BoardData &other) {
				NaiveSimulator::operator=(other);
				return *this;
			}
			override Point generate_move(int color) {return Point(-1, -1);}
		};
		class Minmax : public BoardData
		{
			const static int INFINITE = 100000000;
			const static int MAX_DEPTH = 5;
			const static int TIME_LIMIT = 10;

			int time_limit, count;
			Point nextMove, lastMove;
		public:
			override std::string get_name() { return "MyMinmax"; }
			override std::string get_version() { return "0.3"; }
			override Point generate_move(int color) {
				nextMove = Point(-1, -1);
				lastMove = Point(-2, -2);

				BoardData board(*this);
				time_limit = clock() + TIME_LIMIT * CLOCKS_PER_SEC;
				count = 0;
				int score = alpha_beta_search(board, color, -INFINITE, INFINITE);
				printf("best score: %d, total node: %d\n", score, count);
				return nextMove;
			}

			int alpha_beta_search(BoardData &board, int color, int alpha, int beta, int depth = 0) {
				count++;
				if (depth >= MAX_DEPTH || clock() > time_limit) {
					return evaluate(board, color) - depth; // the quicker, the better
				}
				std::vector<Point> moves = generate_good_moves(board, color);
				if (moves.empty()) {
					if (pass_move(lastMove))
						return evaluate(board, color) - depth;
					moves.push_back(Point(-1, -1));
				}
				for (Point move : moves) {
					//BoardData nextBoard(board);
					board.play_move(move, color);
					lastMove = move;

					int score = -alpha_beta_search(board, other_color(color), -beta, -alpha, depth+1);
					board.undo();
					//nextBoard.undo();
					//if (!(board == nextBoard))
					//	printf("not equal!");

					if (score >= beta)
						return score;
					if (score > alpha) {
						alpha = score;
						if (depth == 0) {
							nextMove = move;
						}
					}
				}

				return alpha;
			}

			std::vector<Point> generate_good_moves(BoardData &board, int color) {
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

				if (moves.empty()) {
					if (legal_moves.size() == board_size*board_size) {
						moves.push_back(Point(board_size/2, board_size/2));
						return moves;
					}
					return legal_moves;
				}
				else
					return moves;
			}
			int evaluate(BoardData &board, int color) {
				int other = other_color(color), score = 0;

				for (int i = 0; i < board_size; i++)
					for (int j = 0; j < board_size; j++)
						if (board.board[i][j] == color)
							score += 10;
						else if (board.board[i][j] == other)
							score -= 10;

				point_set visited;
				for (int i = 0; i < board_size; i++)
					for (int j = 0; j < board_size; j++) {
						Point p(i, j);
						if (visited.find(p) == visited.end())
							if (board.get_board(p) != EMPTY) {
								int factor = (board.get_board(p) == color)?1:-1;
								int count = 0, liberty = 0;
								point_set liberty_set;
								board.map_string(p, [&](Point q){
									count ++;
									visited.insert(q);
									for (int k = 0; k < 4; k++) {
										Point b = q+delta[k];
										if (board.on_board(b) && board.get_board(b) == EMPTY && liberty_set.find(b)==liberty_set.end()) {
											liberty++;
											liberty_set.insert(b);
										}
									}
									return true;
								});
								if (liberty == 1)
									score -= factor*count * 20;
								else if (liberty == 2)
									score -= factor*count * 3;
								else if (liberty > 2)
									score += factor*count * 3;
								score += factor*count * 5;
								score += factor*liberty * 2;
							}
							/*else {
								int count = 0, our = 0, enemy = 0;
								board.flood_fill(p, [&](Point q){
									visited.insert(q);
									if (board.get_board(q) == EMPTY)
										count++;
									else if (board.get_board(q) == color)
										our++;
									else
										enemy++;
									return true;
								});

								int factor = 0;
								if (our == 0 && enemy > 0)
									factor = -1, count += enemy;
								else if (enemy == 0 && our > 0)
									factor = 1, count += our;

								score += factor * count * 10;
							}*/
					}

				return score;
			}
		};
	}
}

#endif