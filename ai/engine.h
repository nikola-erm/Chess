#include <ai/heuristics.h>
#include <board/board.h>

class TEngine {
public:
	TEngine(TBoardBatch& boards);

	int Dfs(int depth, int& cnt);
	int Dfs(int ti, TBoard::TMove* moves, int depth, int a, int b, int& cnt);
	void MakeUserMove();
	void MakeComputerMove();

private:
	const int INF = 1e6;
	TBoardBatch& Boards;

	TBoard::TMove Moves[THREAD_COUNT][1000];
	TBoard::TMove FirstMoves[1000];
	vector<THeuristics> Heuristics;

};