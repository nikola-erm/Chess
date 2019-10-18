#pragma once

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

	vector<int> UseFactors;

	enum EFactors {
		MATERIAL_FACTOR,
		IMMUTABLE_PIECE_FACTOR,
		PAWN_PROGRESS_FACTOR,
		DOUBLE_PAWNS_FACTOR,
		ISOLATED_PAWN_FACTOR,
		BLOCKED_PAWN_FACTOR,
		KNIGHT_ACTIVITY_FACTOR,
		BISHOP_ACTIVITY_FACTOR
	};

	static const vector<int> DefaultUseFactors;

private:
	const TBoard& Board;
	TMask MaskByColor[2];
	EGameStage GameStage;

	int MaterialFactor();
	int ImmutablePiecesFactor();
	int PawnProgressFactor();
	int DoubledPawnsFactor();
	int IsolatedPawnFactor();
	int BlockedPawnFactor();
	int KnightActivityFactor(int wk, int bk);
	int BishopActivityFactor(int wk, int bk);
};