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
      //engine.MakeComputerMoveBetter(1e9, { 11, 6, 0, 1, 0, 2, 3, 0, 4, 1, 1 });
        //engine.MakeComputerMoveBetter(1e9, { 11, 6, 0, 1, 0, 2, 3, 0, 4, 1, 1 });
        //cin.get();
        engine.MakeComputerMoveBetter(1e9, {  9, 4, 1, 0, 1, 0, 1, 1, 2, 1, 1 });
        cin.get();
        //engine.MakeUserMove();
		engine.MakeMonteCarloMove({ 9, 4, 1, 0, 1, 0, 1, 1, 2, 1, 1 });
        //engine.MakeComputerMoveTwoStage({ 9, 4, 1, 0, 1, 0, 1, 1, 2, 1, 1 });
        cin.get();
	}
}