#include <ai/engine.h>
#include <board/board.h>
#include <board/move_serializer.h>
#include <genetic/genetic.h>

#include <util.h>

#include <iostream>

using namespace std;

int main() {
    NBoard::TBoard::StaticInit();
	//RunGenetic();
	//return 0;
	cerr << "Static init complete" << endl;
	NBoard::TBoardBatch boards;
	cerr << "board batch initialized" << endl;
	TEngine engine(boards);
	cerr << "init complete" << endl;
	while (true) {
        engine.MakeComputerMoveBetter(3e9);
        engine.MakeUserMove();
	}
}