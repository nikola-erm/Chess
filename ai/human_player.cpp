#include "human_player.h"

#include <board/move_serializer.h>

#include <iostream>

using namespace NAI;

void THumanPlayer::Undo() {
    Board.Undo();
}

void THumanPlayer::MakeMove(int i) {
    NBoard::TMove moves[100];
    Board.GenerateMoves(moves);
    Board.MakeMove(moves[i]);
}

int THumanPlayer::ChooseMove() {
    NBoard::TMove moves[100];
    int n = Board.GenerateMoves(moves);
	NBoard::TMoveSerializer ms(moves, n, Board);
	int i;
	do {
		string s;
		cout << "Your move: ";
		cin >> s;
		if (s == "sv") {
			ms.ShowValid();
			continue;
		}  else if (s == "print") {
			Board.Print();
			continue;
		}
        i = ms.GetMoveIndex(s);
	} while (i == -1);
    return i;
}