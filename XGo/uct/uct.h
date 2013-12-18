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
#include "policy.h"

namespace Go
{
	namespace UCT {

		template<typename Policy>
		class UCT : public MonteCarlo
		{
			float startTime;
			Policy policy;
			TreeNode *root;
		public:
			UCT():root(NULL) {}
			virtual ~UCT() {
				if (root) {
					delete root;
					root = NULL;
				}
			}
			override std::string get_name() { return "MyUCT"; }
			override std::string get_version() { return "0.2"; }
			override Point generate_move(int color) {
				if (root == NULL)
					root = new TreeNode(Point(-1, -1), color);
				if (root->is_leaf())
					root->expand(*this, policy);

				int cnt = 0;
				long now = clock();
				long limit_time = now + CLOCKS_PER_SEC * (is_first(color)?InitTimeLimit:TimeLimit);

				//for (int i = 0; i < 100; i++)
				while(clock() < limit_time) 
				{
					now = clock();
					cnt++;
					MonteCarlo board(*this);

					TreeNode *p = root;
					int now_color = color;
					while (!p->is_leaf()) {
						p = p->chooseUCBNext();
						board.play_move(p->get_move(), now_color);
						now_color = other_color(now_color);
					}
					if (p == root)
						break;

					float value = p->monteCarlo(board, policy);
					p->update(value>0.5f, value);
					
					if (p->is_mature()) {
						printf("expand...\n");
						p->expand(board, policy);
					}

					printf("round %d, cost time: %f\n", cnt, float(clock()-now)/CLOCKS_PER_SEC);
				}

				log_format("total simulate: %d, total nodes: %d, best val: %f", cnt, root->node_count(), root->value());

				Point move(-1, -1);
				if (!root->is_leaf()) {
					TreeNode *node = root->pickBest();
					move = node->get_move();
				}
				return move;
			}
			override void play_move(Point move, int color) {
				MonteCarlo::play_move(move, color);
				cut_tree(move, color);
			}

			// cut the uct tree with current step
			void cut_tree(Point move, int color) {
				if (root == NULL)
					return;

				TreeNode *node = NULL;
				if (root->color == color) {
					node = root->get_child(move);
					if (node != NULL) {
						root->remove(node);
						node->parent = NULL;
					}
				}
				
				delete root;
				// set new root
				root = node;
			}
		};
	}
}

#endif