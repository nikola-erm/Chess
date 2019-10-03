#include <board/board.h>

#include <util.h>

class THeuristics {
public:
	enum EGameStage {
		GS_OPENNING,
		GS_MIDDLE,
		GS_ENDING
	};

	THeuristics(const TBoard& board);	
	void UpdateGameStage();
	int GetScore();

private:
	const TBoard& Board;
	EGameStage GameStage;

	int MaterialFactor();
	int ImmutablePiecesFactor();
	int PawnProgressFactor();
	int DoubledPawnsFactor();
	int IsolatedPawnFactor();
};