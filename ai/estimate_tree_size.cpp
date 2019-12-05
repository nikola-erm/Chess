#include "estimate_tree_size.h"

#include <util.h>

#include <iostream> 

static TBoard::TMove moves[100];

vector<double> EstimateTreeSize(TBoard& board, int depth, int itrCnt) {
    //cerr << "Estimate tree size";
    vector<TBoard::TMove> story(depth);
    vector<double> ans(depth + 1, 0.0);
    ans[0] = itrCnt;
    for (int itr = 0; itr < itrCnt; itr++) {
        //cerr << ".";
        double c = 1.0;
        int maxd = -1;
        for (int d = 0; d < depth; d++) {
            vector<int> iovs;
            int n = board.GenerateMovesUnchecked(moves) - moves;
            for (int i = 0; i < n; i++) {
                board.MakeMove(moves[i]);
                if (!board.IsOpKingUnderAttack())
                    iovs.push_back(i);
                board.UndoMove(moves[i]);
            }
            if (iovs.empty())
                break;
            maxd = d;
            int i = iovs[Rand(iovs.size())];
            story[d] = moves[i];
            c *= iovs.size();
            ans[d+1] += c;
            board.MakeMove(story[d]);
        }
        for (; maxd >= 0; maxd--)
            board.UndoMove(story[maxd]);
    }
    //cerr << endl;
    for (int i = 0; i < ans.size(); i++) {
        ans[i] /= itrCnt;
        if (ans[i] < 1.0)
            ans[i] = 1.0;
        //cerr << ans[i] << " ";
    }
    //cerr << endl;
    return ans;
}