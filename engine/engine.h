#pragma once

namespace NEngine {

class IPlayer {
public:
    virtual void Undo() = 0;
    virtual int ChooseMove() = 0;
    virtual void MakeMove(int i) = 0;

    int ChooseAndMakeMove();
};

class TEngine {
public:
    int Play(IPlayer* wp, IPlayer* bp);

};

} //NEngine