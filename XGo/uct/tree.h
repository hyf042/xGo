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
#define NODE_FOR_EACH_CHILD(node, act_node, block) TreeNode *_p = child; while(_p) { \
		TreeNode &act_node = *_p; \
		block; \
		_p = _p->brother; \
	}

		const static int TimeLimit = 1;
		const static int InitTimeLimit = 1;
		const static int ExpandThrehold = 2;
		const static float ExploreRate = 2.0f;

		class TreeNode
		{
		public:
			const static int MAX_UCB = 10000;

			int color;
			float val;
			int win;
			int playCnt;
			TreeNode *parent;
			Point move;
		public:
			TreeNode *child, *brother;
			TreeNode(Point move, int color)
				:child(NULL), brother(NULL), parent(NULL), color(color), move(move) {
				win = 0;
				val = 0;
				playCnt = 0;
			}
			~TreeNode () {
				TreeNode *p = child;
				while(p) {
					TreeNode *q = p;
					p = p->brother;
					delete q;
				}
			}

			float value(bool onlyWin = true) const {
				if (onlyWin)
					return float(win)/playCnt;
				else
					return val;
			}
			float UCB() const {
				if (playCnt == 0)
					return MAX_UCB + rand() % 1000;
				return value() + std::sqrt(ExploreRate * std::log(parent->playCnt) / playCnt);
			}
			bool is_root() const {
				return parent == NULL;
			}
			bool is_leaf() const {
				return child == NULL;
			}
			bool is_mature() const {
				return playCnt > ExpandThrehold;
			}
			TreeNode *get_father() const {return parent;}
			Point get_move() const {return move;}
			TreeNode* get_child(Point move) const {
				NODE_FOR_EACH_CHILD(this, node, {
					if (node.move == move)
						return &node;
				});
				return NULL;
			}

			void append(TreeNode *node) {
				if (child == NULL)
					child = node;
				else {
					node->brother = child;
					child = node;
				}
				node->parent = this;
			}
			void remove(TreeNode *node) {
				TreeNode *p = child;
				if (p == node)
					child = p->brother;
				else {
					TreeNode *q = p->brother;
					while(q) {
						if (q == node) {
							p->brother = q->brother;
							break;
						}
						p = q, q = p->brother;
					}
				}
			}
			int node_count() {
				int cnt = 1;
				NODE_FOR_EACH_CHILD(this, node, {
					cnt += node.node_count();
				});
				return cnt;
			}

			// update the val throughout to root (maximum of all children) & win cnt
			void update(bool isWin, float newVal) {
				if (isWin)
					win++;
				if (!is_leaf()) {
					val = 0;
					NODE_FOR_EACH_CHILD(this, node, {
						if (node.playCnt > 0)
							val = std::max(val, 1 - node.val);
					});
				}
				else
					val = (playCnt * val + newVal) / (playCnt+1);
				playCnt++;

				if (parent != NULL)
					parent->update(!isWin, newVal);
			}

			// expand this node with good_moves
			void expand(MonteCarlo &board, Policy &policy) {
				if (!is_leaf())
					return;

				std::vector<Point> moves = policy.generate_good_moves(board, color);
				for (auto move : moves) {
					TreeNode *node = new TreeNode(move, board.other_color(color));
					append(node);
				}
			}
			// pick largest ucb subtree
			TreeNode* chooseUCBNext() {
				if (is_leaf()) return this;

				TreeNode *ret = NULL;
				float maxVal;

				NODE_FOR_EACH_CHILD(this, node, {
					float tmp = node.UCB();
					if (ret == NULL || tmp > maxVal) {
						ret = &node;
						maxVal = tmp;
					}
				});

				return ret;
			}
			// pick best val subtree (must played)
			TreeNode *pickBest() {
				if (is_leaf()) return NULL;

				TreeNode *ret = NULL;
				float maxVal = 0;

				NODE_FOR_EACH_CHILD(this, node, {
					if (node.playCnt > 0) {
						float tmp = 1 - node.value();
						if (ret == NULL || tmp > maxVal) {
							ret = &node;
							maxVal = tmp;
						}
					}
				});

				return ret;
			}
			// simulate the board with monteCarlo algorithm
			float monteCarlo(MonteCarlo &board, Policy &policy) {
				MonteCarlo simulator(board);
				return simulator.simulate(policy, color);
			}
		};
	}
}

#endif