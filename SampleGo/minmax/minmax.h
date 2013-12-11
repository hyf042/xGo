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
			const static int MAX_DEPTH = 4;
			const static int TIME_LIMIT = 10;

			int time_limit;
			Point nextMove, lastMove;
		public:
			override std::string get_name() { return "MyMinmax"; }
			override std::string get_version() { return "0.1"; }
			override Point generate_move(int color) {
				nextMove = Point(-1, -1);
				lastMove = Point(-2, -2);

				BoardData board(*this);
				time_limit = clock() + TIME_LIMIT * CLOCKS_PER_SEC;
				int score = alpha_beta_search(board, color, -INFINITE, INFINITE);
				printf("best score: %d\n", score);
				return nextMove;
			}

			int alpha_beta_search(BoardData &board, int color, int alpha, int beta, int depth = 0) {
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
					BoardData nextBoard(board);
					nextBoard.play_move(move, color);
					lastMove = move;

					int score = -alpha_beta_search(nextBoard, other_color(color), -beta, -alpha, depth+1);
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

				if (moves.empty())
					return legal_moves;
				else
					return moves;
			}
			int evaluate(BoardData board, int color) {
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
						if (visited.find(p) == visited.end() && board.get_board(p) != EMPTY) {
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
					}

				return score;
			}
		};
	}
}

#endif