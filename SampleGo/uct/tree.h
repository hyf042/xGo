/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _UCT_TREE_H_
#define _UCT_TREE_H_

#include <cmath>
#include "../naiveSimulator.h"
#include "MonteCarlo.h"

namespace Go
{
	namespace UCT 
	{
		class TreeNode
		{
			friend class UCT;
			const static int MAX_UCB = 10000;

			int color;
			float val;
			int playCnt;
			TreeNode *father;
			MonteCarlo data;
			Point move;
		public:
			TreeNode *child, *brother;
			TreeNode(MonteCarlo *data, int color)
				:child(NULL), brother(NULL), father(NULL), color(color) {
				val = 0;
				playCnt = 0;
				this->data = *data;
			}
			~TreeNode () {
				TreeNode *p = child;
				while(p) {
					TreeNode *q = p;
					p = p->brother;
					delete q;
				}
			}

			bool is_leaf() const {
				return child == NULL;
			}
			TreeNode *get_father() const {return father;}
			Point get_move() const {return move;}

			float get_UCB() const {
				if (playCnt == 0)
					return MAX_UCB + rand() % 1000;
				return val + std::sqrt(2 * std::log(father->playCnt) / playCnt);
			}

			void append(TreeNode *node) {
				if (child == NULL)
					child = node;
				else {
					node->brother = child;
					child = node;
				}
				node->father = this;
			}
			int count() {
				int cnt = 1;
				TreeNode *p = child;
				while(p) {
					cnt += p->count();
					p = p->brother;
				}
				return cnt;
			}

			// update the val (maximum of all children)
			void update() {
				if (!is_leaf()) {
					TreeNode *p = child;
					val = 0, playCnt ++;

					while(p) {
						if (p->playCnt > 0)
							val = std::max(val, 1 - p->val);
						p = p->brother;
					}
				}
				if (father != NULL)
					father->update();
			}
			// expand this node with good_moves
			void expand() {
				if (!is_leaf())
					return;

				std::vector<Point> moves = data.generate_good_moves(color);
				for (auto move : moves) {
					TreeNode *node = new TreeNode(&data, data.other_color(color));
					node->data.play_move(move, color);
					node->move = move;
					append(node);
				}
			}
			// pick largest ucb subtree
			TreeNode* chooseUCBNext() {
				if (is_leaf()) return this;

				TreeNode *p = child;
				TreeNode *ret = NULL;
				float maxVal;

				while (p) {
					float tmp = p->get_UCB();

					if (ret == NULL || tmp > maxVal) {
						ret = p;
						maxVal = tmp;
					}
					p = p->brother;
				}

				if (ret == NULL)
					return this;
				else
					return ret->chooseUCBNext();
			}
			// pick best val subtree (must played)
			TreeNode *pickBest() {
				if (is_leaf()) return NULL;

				TreeNode *p = child;
				TreeNode *ret = NULL;
				float maxVal = 0;

				while (p) {
					if (p->playCnt > 0) {
						float tmp = 1 - p->val;
						if (ret == NULL || tmp > maxVal) {
							ret = p;
							maxVal = tmp;
						}
					}
					p = p->brother;
				}

				return ret;
			}
			// simulate the board with monteCarlo algorithm
			float monteCarlo() {
				MonteCarlo simulator(data);
				float tmp = simulator.simulate(color);
				val = (val * playCnt + tmp) / (playCnt + 1);
				playCnt++;
				return tmp;
			}
		};
	}
}

#endif