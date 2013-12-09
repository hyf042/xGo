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
			int color;
			float val;
			int playCnt;
			MonteCarlo data;
		public:
			TreeNode *child, *brother;
			TreeNode(int board_size, short board[MAX_BOARD][MAX_BOARD], int color):child(NULL), brother(NULL) {
				val = 0;
				playCnt = 0;
				data.set_board(board_size, board);
			}

			bool is_leaf() const {
				return child == NULL;
			}

			float get_UCB(int total) const {
				return val + std::sqrt(2 * std::log(total) / playCnt);
			}

			void append(TreeNode *node) {
				if (child == NULL)
					child = node;
				else {
					TreeNode *p = child;
					while(p->brother != NULL) p = p->brother;
					p->brother = node;
				}
			}

			void expand() {
				if (!is_leaf())
					return;

				std::vector<Point> moves = data.generate_good_moves(color);
				for (auto move : moves) {
					TreeNode *node = new TreeNode(data.board_size, data.board, data.other_color(color));
					node->data.play_move(move, data.other_color(color));
					append(node);
				}
			}
			TreeNode* choose() {
				if (is_leaf()) return NULL;

				TreeNode *p = child;
				TreeNode *ret = NULL;
				float maxVal;

				while (p) {
					float tmp = p->get_UCB(playCnt);
					if (ret == NULL || tmp > maxVal) {
						ret = p;
						maxVal = tmp;
					}
					p = p->brother;
				}

				return ret;
			}
		};
	}
}

#endif