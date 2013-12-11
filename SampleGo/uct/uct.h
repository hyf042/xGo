/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _UCT_H_
#define _UCT_H_

#include <cstdlib>
#include <ctime>
#include "../naiveSimulator.h"
#include "tree.h"

namespace Go
{
	namespace UCT {
		class UCT : public MonteCarlo
		{
			const static int TimeLimit = 5;
			const static int ExpandThrehold = 3;
			float startTime;
		public:
			override std::string get_name() { return "MyUCT"; }
			override Point generate_move(int color) {
				TreeNode *root = new TreeNode(this, color);
				root->expand();

				int cnt = 0;
				long now = clock();
				long limit_time = now + CLOCKS_PER_SEC * TimeLimit;

				//for (int i = 0; i < 100; i++)
				while(clock() < limit_time) 
				{
					now = clock();
					cnt ++;

					TreeNode *p = root;
					p = p->chooseUCBNext();
					if (p == root)
						break;

					p->monteCarlo();
					p->update();
					
					if (p->playCnt >= ExpandThrehold)
						p->expand();

					printf("round %d, cost time: %f\n", cnt, float(clock()-now)/CLOCKS_PER_SEC);
				}

				log_format("total simulate: %d, total nodes: %d, best val: %f", cnt, root->count(), root->val);

				Point move(-1, -1);
				if (!root->is_leaf()) {
					TreeNode *node = root->pickBest();
					move = node->get_move();
				}
				delete root;
				return move;
			}
		};
	}
}

#endif