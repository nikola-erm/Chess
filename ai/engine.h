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

    struct TDfsResult {
        int Score;
        int StumbleScore;

        TDfsResult GetNormalized() const;
    };

	TEngine(TBoardBatch& boards);

	TDfsResult Dfs(int depth, int& cnt, int a = -INF, int b = INF);
	TDfsResult Dfs(int ti, TBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt, bool opTurn);
    int DfsFixed(int ti, TBoard::TMove* moves, int depth, int& cnt, int a);
	void MakeUserMove();
	void MakeComputerMove(int posCntLim);
    void MakeComputerMoveBetter(int posCntLim);
	void Print() const;

private:
	TBoardBatch& Boards;

	TBoard::TMove Moves[THREAD_COUNT][1000];
	TBoard::TMove FirstMoves[1000];
	vector<THeuristics> Heuristics;

};

TEngine::TDfsResult operator -(const TEngine::TDfsResult& score);