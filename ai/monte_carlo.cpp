#include <util.h>
#include "monte_carlo.h"

#include <board/move_serializer.h>

using namespace NBoard;

int TMonteCarlo::ChooseMove(int v, int n) {
    vector<int> ioz;
    for (int i = 0; i < n; i++) {
        if (Data[v + 2 + i] == 0)
            ioz.push_back(i);
    }
    if (!ioz.empty())
        return ioz[Rand(ioz.size())];
    int minScore = 10000, maxScore = -10000;
    for (int i = 0; i < n; i++) {
        minScore = min(minScore, Data[Data[v + 2 + i] + 1]);
        maxScore = max(maxScore, Data[Data[v + 2 + i] + 1]);
    }
    maxScore += (1 + maxScore - minScore);
    static int psum[100];
    psum[0] = (maxScore - Data[Data[v + 2 + 0] + 1]); // / Log2(Data[Data[v + 2 + 0]]) + 1;
    for (int i = 1; i < n; i++)
        psum[i] = psum[i-1] + (maxScore - Data[Data[v + 2 + i] + 1]); // / Log2(Data[Data[v + 2 + i]]) + 1;
    int tsum = Rand(psum[n-1]);
    int ret = 0;
    while (tsum >= psum[ret])
        ret++;
    return ret;
}

int TMonteCarlo::Dfs(int v, TBoard& board, THeuristics& heuristics, int depth) {
    bool newV = false;
    if (v == DataCount) {
        DataCount += 2;
        Data[v] = 0;
        newV = true;
    }
    Data[v]++;
    
    auto& moves = Moves[depth];
    int n = board.GenerateMoves(moves);
    if (newV) {
        DataCount += n;
        for (int i = 0; i < n; i++)
            Data[v + 2 + i] = 0;
    }
    if (n == 0)
        return Data[v+1] = board.IsMyKingUnderAttack() ? -1000 : 0;
    if (newV)
        return Data[v+1] = heuristics.GetScore();
    int mi = ChooseMove(v, n);
    board.MakeMove(moves[mi]);
    int& nxtV = Data[v + 2 + mi];
    if (nxtV == 0)
        nxtV = DataCount;
    Dfs(nxtV, board, heuristics, depth+1);
    board.UndoMove(moves[mi]);
    Data[v+1] = -Data[nxtV+1];
    for (int i = 0; i < n; i++)
        if (Data[v + 2 + i] != 0)
            Data[v+1] = max(Data[v+1], -Data[Data[v+2+i] + 1]);
    return Data[v+1];
}

bool TMonteCarlo::MakeMove(TBoard& board, THeuristics& heuristics, TMove& bestMove, string& moveName) {
    DataCount = 0;
    int n = board.GenerateMoves(Moves[0]);
    if (n == 0)
        return false;
    for (int itr = 0; itr < ItrCount; itr++) {
        Dfs(0, board, heuristics, 0);
        if ((itr + 1) % 100 == 0)
            cerr << ((itr + 1) * 100) / ItrCount << "% score=" << Data[1] << " tsize=" << DataCount << "                     \r";
    }
    cerr << endl;
    TMoveSerializer ms(Moves[0], n, board);
    vector<int> iobs;
    for (int i = 0; i < n; i++) {
        if (Data[2 + i] == 0)
            continue;
        if (iobs.empty() || Data[Data[2 + i] + 1] < Data[Data[2 + iobs[0]] + 1]) {
            iobs.clear();
            iobs.push_back(i);
        } else if (Data[Data[2 + i] + 1] == Data[Data[2 + iobs[0]] + 1]) {
            iobs.push_back(i);
        }
    } 
    int iob = iobs[Rand(iobs.size())];
    bestMove = Moves[0][iob];
    moveName = ms.GetMoveName(iob);
    return true;
}

TMonteCarlo::TMonteCarlo() {
    Data.resize(100000000);
}