#pragma once

#include <ai/heuristics.h>
#include <board/board.h>
#include <config.h>
#include <engine/engine.h>

namespace NAI {

class TBinSearchNoCachePlayer : public NEngine::IPlayer {
public:
    TBinSearchNoCachePlayer(double posCntLim);

    virtual void Undo() override;
    virtual int ChooseMove() override;
    virtual void MakeMove(int i) override;

private:
    int DfsFixed(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a);

    NBoard::TBoardBatch Boards;
	NBoard::TMove Moves[THREAD_COUNT][1000];
	NBoard::TMove FirstMoves[1000];
	vector<THeuristics> Heuristics;

    const double PosCntLim;
};

}