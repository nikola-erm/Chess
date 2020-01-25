#include "engine.h"

#include <board/board.h>
#include <board/move_serializer.h>

#include <iostream>

using namespace NEngine;

int TEngine::Play(IPlayer* whites, IPlayer* blacks) {
    NBoard::TBoard board;
    NBoard::TMove moves[100];
    
    while (true) {
        board.UpdateStatus();
        if (board.Status != NBoard::GS_PLAY)
            break;
        auto me = (board.GetTurn() & 1) ? blacks : whites;
        auto op = (board.GetTurn() & 1) ? whites : blacks;

        string s;
        board.PrintStory();
        //getline(cin, s);
        if (s == "story") {
            board.PrintStory();
            continue;
        }
        if (s == "undo") {
            board.Undo();
            me->Undo();
            op->Undo();
            continue;
        }
        
        int n = board.GenerateMoves(moves);
        NBoard::TMoveSerializer ms(moves, n, board);

        int i = me->ChooseAndMakeMove();
        op->MakeMove(i);
        board.MakeMove(moves[i], ms.GetMoveName(i));
    }
    if (board.Status == NBoard::GS_DRAW)
        return 0;
    return (board.GetTurn() & 1) ? 1 : 0;
}

int IPlayer::ChooseAndMakeMove() {
    int i = ChooseMove();
    MakeMove(i);
    return i;
}