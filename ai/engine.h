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

	TEngine(NBoard::TBoardBatch& boards);

	TDfsResult Dfs(int depth, int& cnt, int a = -INF, int b = INF);
	TDfsResult Dfs(int ti, NBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt, bool opTurn);
    int DfsFixed(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a);
    int DfsIterable(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a);
	void MakeUserMove();
	void MakeComputerMove(int posCntLim, vector<int> useFactors = THeuristics::DefaultUseFactors);
    void MakeComputerMoveBetter(long long posCntLim, vector<int> useFactors = THeuristics::DefaultUseFactors);
	void Print() const;
	void MakeSpecialAction();

private:
	NBoard::TBoardBatch& Boards;

	NBoard::TMove Moves[THREAD_COUNT][1000];
	NBoard::TMove FirstMoves[1000];
	vector<THeuristics> Heuristics;

};

TEngine::TDfsResult operator -(const TEngine::TDfsResult& score);