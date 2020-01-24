#include "monte_carlo.h"

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
	
	struct TGameTreeNode {
        NBoard::TMove Move;
        string MoveName;
        int MinScore;
        int MaxScore;
        vector<TGameTreeNode> Children;
        
        void Print() const;
        int CalcNumberOfInteresting(int a, int b) const;
        bool IsTrivial(int a, int b) const;
    };
	
	TEngine(NBoard::TBoardBatch& boards);

	TDfsResult Dfs(int depth, int& cnt, int a = -INF, int b = INF);
	TDfsResult Dfs(int ti, NBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt, bool opTurn);
    int DfsFixed(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a);
    int DfsIterable(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a);
	void MakeUserMove();
	void MakeComputerMove(int posCntLim, vector<int> useFactors = THeuristics::DefaultUseFactors);
    void MakeComputerMoveBetter(long long posCntLim, vector<int> useFactors = THeuristics::DefaultUseFactors);
	void MakeComputerMoveTwoStage(vector<int> useFactors);
    void Print() const;
	void MakeSpecialAction();
    void MakeMonteCarloMove(vector<int> useFactors = THeuristics::DefaultUseFactors);

private:
	NBoard::TBoardBatch& Boards;

	NBoard::TMove Moves[THREAD_COUNT][1000];
	NBoard::TMove FirstMoves[1000];
	vector<THeuristics> Heuristics;

    void BuildGameTreeStructure(TGameTreeNode& v, int depth);
    void BuildLeafList(TGameTreeNode& v, vector<vector<TGameTreeNode*>>& result, vector<TGameTreeNode*>& leafStack);
    int CalcReEstimateLeafs(const TGameTreeNode& v, int minScore, int maxScore);
    TGameTreeNode BuildGameTree();
    void BuildLeafsForUpdateList(TGameTreeNode& v, vector<vector<TGameTreeNode*>>& result, vector<TGameTreeNode*>& leafStack, int a);
    void EstimateLeafs(vector<vector<TGameTreeNode*>>& leafs, int depth, int a);
	bool NeedEstimate(const vector<TGameTreeNode*>& leafs, int a);
    int DfsFixedScore(int depth, int ti = 0);
    int DfsFixedScore(int depth, int ti, int a, int b);
    bool IsTrivial(const TGameTreeNode& v, int minScore, int maxScore);
    void RelaxScores(TGameTreeNode& v, int minScore, int maxScore);
    void Explain(const TGameTreeNode& v, int d = 0);

    TMonteCarlo MonteCarlo;
};

TEngine::TDfsResult operator -(const TEngine::TDfsResult& score);