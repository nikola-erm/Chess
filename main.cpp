#include <ai/engine.h>
#include <board/board.h>
#include <board/move_serializer.h>
#include <genetic/genetic.h>

#include <util.h>

#include <iostream>

using namespace std;

int main() {
    TBoard::StaticInit();
	//RunGenetic();
	//return 0;
	cerr << "Static init complete" << endl;
	TBoardBatch boards;
	cerr << "board batch initialized" << endl;
	TEngine engine(boards);
	cerr << "init complete" << endl;
	while (true) {
		engine.MakeUserMove();
		//engine.MakeSpecialAction();
		//engine.MakeComputerMove(1e6, { 9, 0, 1, 0, 1, 1, 1, 0 });
		//engine.MakeSpecialAction();
		engine.MakeComputerMove(2e6, THeuristics::DefaultUseFactors);
	}
}