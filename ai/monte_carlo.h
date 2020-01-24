#include <ai/heuristics.h>
#include <board/board.h>

class TMonteCarlo {
public:
    TMonteCarlo();
    bool MakeMove(NBoard::TBoard& board, THeuristics& heuristics, NBoard::TMove& bestMove, string& moveName);

private:
    NBoard::TMove Moves[10][100];

    vector<int> Data;
    int DataCount;
    const int MaxDepth = 10;
    const int ItrCount = 100000;

    int Dfs(int v, NBoard::TBoard& board, THeuristics& heuristics, int depth);
    int ChooseMove(int v, int n);

};