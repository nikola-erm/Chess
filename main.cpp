#include <ai/engine.h>
#include <board/board.h>
#include <board/move_serializer.h>

#include <util.h>

#include <iostream>

using namespace std;

TBoard::TMove moves[100];

int main() {
	TBoard::StaticInit();
	cerr << "Static init complete" << endl;
	TBoardBatch boards;
	cerr << "board batch initialized" << endl;
	TEngine engine(boards);
	cerr << "init complete" << endl;
	while (true) {
		engine.MakeUserMove();
		engine.MakeComputerMove(3e5);
	}
}