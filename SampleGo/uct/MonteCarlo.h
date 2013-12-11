/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _UCT_MONTE_CARLO_H_
#define _UCT_MONTE_CARLO_H_

#include <algorithm>
#include "../naiveSimulator.h"
#include "policy.h"

namespace Go
{
	namespace UCT {
		class MonteCarlo : public NaiveSimulator
		{
			friend class TreeNode;
			
			static const int MAX_SIMULATE_STEP = 1000;
		public:
			MonteCarlo() {}
			MonteCarlo(MonteCarlo &other):NaiveSimulator(other) {}
			MonteCarlo& operator=(MonteCarlo &other) {
				NaiveSimulator::operator=(other);
				return *this;
			}

			override std::string get_name() { return "MonteCarlo"; }
			override Point generate_move(int color) {
				/*MonteCarlo simulator;
				
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
				}*/

				return Point(-1, -1);
			}
		public:
			float simulate(Policy &policy, int color) {
				int now_color = color;
				bool isPass = false;
				//int sum = 0, simulateCnt = 0;
				for (int i = 0; i < MAX_SIMULATE_STEP; i++) {
					//int now = clock();
					//simulateCnt++;
					Point move = policy.next_simulate(*this, now_color);
					//sum += clock()-now;
					if (pass_move(move)) {
						if (isPass)
							break;
						else
							isPass = true;
					}
					else {
						isPass = false;
						
						play_move(move, now_color);
					}
					now_color = other_color(now_color);
				}
				//printf("generate time cost: %f (%d)\n", (float)sum/CLOCKS_PER_SEC, simulateCnt);

				return policy.evaluate(*this, color);
			}
		};
	}
}

#endif