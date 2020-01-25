#pragma once

#include <board/board.h>
#include <engine/engine.h>

namespace NAI {

class THumanPlayer : public NEngine::IPlayer {
public:
    virtual void Undo() override;
    virtual void MakeMove(int i) override;
    virtual int ChooseMove() override;

private:
    NBoard::TBoard Board;    
};

}