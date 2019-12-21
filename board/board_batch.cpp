#include "board.h"

using namespace NBoard;

TBoard& TBoardBatch::operator[](int i) {
	return Boards[i];
}

TMove* TBoardBatch::GenerateMovesUnchecked(TMove* moves) {
	return Boards[0].GenerateMovesUnchecked(moves);
}

TBoardBatch::TBoardBatch() {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards.emplace_back();
}

TBoardBatch::TBoardBatch(const string& fen) {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards.emplace_back(fen);
}

void TBoardBatch::MakeMove(const TMove& m) {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards[i].MakeMove(m);
}

void TBoardBatch::MakeMove(const TMove& m, const string& s) {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards[i].MakeMove(m, s);
}

void TBoardBatch::UndoMove(const TMove& m) {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards[i].UndoMove(m);
}

void TBoardBatch::PrintStory() const {
	Boards[0].PrintStory();
}

void TBoardBatch::Undo() {
	for (int i = 0; i < THREAD_COUNT; i++)
		Boards[i].Undo();
}