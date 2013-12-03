/*
*	Author:	hyf042
*	Date:	12-02-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _BROWN_H_
#define _BROWN_H_

#include "naiveSimulator.h"

namespace Go
{
	class Brown : public NaiveSimulator
	{
	public:
		override Point generate_move(int color) {
			std::vector<Point> moves;

			for (int ai = 0; ai < board_size; ai++)
				for (int aj = 0; aj < board_size; aj++) {
					Point a(ai, aj);
					/* Consider moving at (ai, aj) if it is legal and not suicide. */
					if (legal_move(a, color)
						&& !suicide(a, color)) {
							/* Further require the move not to be suicide for the opponent... */
							if (!suicide(a, other_color(color)))
								moves.push_back(a);
							else {
								/* ...however, if the move captures at least one stone,
								* consider it anyway.
								*/
								for (int k = 0; k < 4; k++) {
									Point b = a+delta[k];
									if (on_board(b) && get_board(b) == other_color(color)) {
										moves.push_back(a);
										break;
									}
								}
							}
					}
				}

			/* Choose one of the considered moves randomly with uniform
			* distribution. (Strictly speaking the moves with smaller 1D
			* coordinates tend to have a very slightly higher probability to be
			* chosen, but for all practical purposes we get a uniform
			* distribution.)
			*/
			Point move = Point(-1, -1);
			std::vector<Point> best;
			if (moves.size() > 0) {
				int max_captured = -1;
				for (int i = 0; i < moves.size(); i++) {
					Point p = moves[i];
					int sum_captured = 0;
					for (int k = 0; k < 4; k++) {
						Point a = p+delta[k];
						if (on_board(a)
							&& get_board(a) == other_color(color)
							&& !has_additional_liberty(a, p))
							sum_captured += count_string(a);
					}

					if (sum_captured > max_captured) {
						max_captured = sum_captured;
						best.clear();
						best.push_back(p);
					}
					else if (sum_captured == max_captured)
						best.push_back(p);
				}

				move = best[rand()%best.size()];
			}

			return move;
		}
	};
}

#endif