/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _EATER_H_
#define _EATER_H_

#include "naiveSimulator.h"

namespace Go
{
	class Eater : public NaiveSimulator
	{
	public:
		override std::string get_name() { return "eater"; }
		override Point generate_move(int color) {
			std::vector<Point> moves = generate_legal_moves(color);
			Point move = Point(-1, -1);
			int min_liberty = board_size*board_size;

			if (moves.size() > 0) {
				for (auto p : moves)
				{
					for (int k = 0; k < 4; k++) {
						Point a = p+delta[k];
						if (on_board(a)
							&& get_board(a) == other_color(color)) {
								int liberty = count_liberty(a);
								if (liberty < min_liberty || liberty == min_liberty && pos_evaluate(p) > pos_evaluate(move)) {
									move = p;
									min_liberty = liberty;
								}
						}
					}
				}
				if (move == Point(-1, -1))
					move = moves[rand()%moves.size()];
			}
			log_format("best (%d,%d) -> %d", move.r, move.c, min_liberty);

			return move;
		}
	};
}

#endif