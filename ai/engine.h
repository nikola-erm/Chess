#include <ai/heuristics.h>
#include <board/board.h>
#include <util.h>

class TEngine {
public:
	struct TDfsLimits {
		int Depth;
		int Capture;
		int Force;
	};

	TEngine(TBoardBatch& boards);

	int Dfs(int depth, int& cnt, int a = -INF, int b = INF);
	int Dfs(int ti, TBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt);
	void MakeUserMove();
	void MakeComputerMove(int posCntLim);
	void Print() const;

private:
	TBoardBatch& Boards;

	TBoard::TMove Moves[THREAD_COUNT][1000];
	TBoard::TMove FirstMoves[1000];
	vector<THeuristics> Heuristics;

};