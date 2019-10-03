#include "board.h"

TBoard& TBoardBatch::operator[](int i) {
	return Boards[i];
}

TBoard::TMove* TBoardBatch::GenerateMovesUnchecked(TBoard::TMove* moves) {
	return Boards[0].GenerateMovesUnchecked(moves);
}

TBoardBatch::TBoardBatch() {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards.emplace_back();
}

void TBoardBatch::MakeMove(const TBoard::TMove& m) {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards[i].MakeMove(m);
}

void TBoardBatch::UndoMove(const TBoard::TMove& m) {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards[i].UndoMove(m);
}