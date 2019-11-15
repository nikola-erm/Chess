#include "heuristics.h"



THeuristics::THeuristics(const TBoard& board)
	: Board(board) {}

void THeuristics::UpdateGameStage() {
	if (Board.Turn < 20) {
		GameStage = GS_OPENNING;
	} else {
		for (int i = 0; i < 2; i++)
			MaskByColor[i] = Board.Masks[TBoard::MT_WQUEEN + i]
				| Board.Masks[TBoard::MT_WROOK + i]
				| Board.Masks[TBoard::MT_WBISHOP + i]
				| Board.Masks[TBoard::MT_WKNIGHT + i]
				| Board.Masks[TBoard::MT_WPAWN + i]
				| Board.Masks[TBoard::MT_WKING + i];
		GameStage = BCNT(MaskByColor[0] | MaskByColor[1]) < 16 ? GS_ENDING : GS_MIDDLE;
	}
}

int THeuristics::GetScore() {
	UpdateGameStage();
	int wk = GetBitPos(Board.Masks[TBoard::MT_WKING]);
	int bk = GetBitPos(Board.Masks[TBoard::MT_BKING]);
	int score = 0;
	if (UseFactors[MATERIAL_FACTOR])
		score += MaterialFactor() * UseFactors[MATERIAL_FACTOR];
	if (UseFactors[IMMUTABLE_PIECE_FACTOR])
		score += ImmutablePiecesFactor() * UseFactors[IMMUTABLE_PIECE_FACTOR];
	if (UseFactors[PAWN_PROGRESS_FACTOR])
		score += PawnProgressFactor() * UseFactors[PAWN_PROGRESS_FACTOR];
	if (UseFactors[DOUBLE_PAWNS_FACTOR])
		score += DoubledPawnsFactor() * UseFactors[DOUBLE_PAWNS_FACTOR];
	if (UseFactors[ISOLATED_PAWN_FACTOR])
		score += IsolatedPawnFactor() * UseFactors[ISOLATED_PAWN_FACTOR];
	if (UseFactors[BLOCKED_PAWN_FACTOR])
		score += BlockedPawnFactor() * UseFactors[BLOCKED_PAWN_FACTOR];
	if (UseFactors[KNIGHT_ACTIVITY_FACTOR])
		score += KnightActivityFactor(wk, bk) * UseFactors[KNIGHT_ACTIVITY_FACTOR];
	if (UseFactors[BISHOP_ACTIVITY_FACTOR])
		score += BishopActivityFactor(wk, bk) * UseFactors[BISHOP_ACTIVITY_FACTOR];
	if (UseFactors[KING_CENTRALITY_FACTOR])
		score += KingCentralityFactor(wk, bk) * UseFactors[KING_CENTRALITY_FACTOR];
	//if (UseFactors[DOUBLE_ROOK_FACTOR])
	//	score += DoubleRookFactor() * UseFactors[DOUBLE_ROOK_FACTOR];
	return score;
}

int THeuristics::IsolatedPawnFactor() {
	int wm = 0, bm = 0;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnA) wm |= 1;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnA) bm |= 1;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnB) wm |= 2;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnB) bm |= 2;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnC) wm |= 4;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnC) bm |= 4;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnD) wm |= 8;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnD) bm |= 8;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnE) wm |= 16;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnE) bm |= 16;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnF) wm |= 32;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnF) bm |= 32;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnG) wm |= 64;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnG) bm |= 64;
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumnH) wm |= 128;
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumnH) bm |= 128;
	if (Board.Turn & 1)
		return TBoard::IsolatedPawnCount[wm] - TBoard::IsolatedPawnCount[bm];
	else
		return TBoard::IsolatedPawnCount[bm] - TBoard::IsolatedPawnCount[wm];
}

int THeuristics::DoubledPawnsFactor() {
	int score = 0;
#define DOUBLE_PAWN_FACTOR_FOR_COLUMN(c) \
	if (Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumn##c) score += 1 - BCNT(Board.Masks[TBoard::MT_WPAWN] & TBoard::MaskColumn##c); \
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumn##c) score -= 1 - BCNT(Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumn##c);
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(A)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(B)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(C)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(D)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(E)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(F)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(G)
	DOUBLE_PAWN_FACTOR_FOR_COLUMN(H)
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::MaterialFactor() {
	int sumw = BCNT(Board.Masks[TBoard::MT_WQUEEN]) * 9
			 + BCNT(Board.Masks[TBoard::MT_WROOK]) * 5
			 + BCNT(Board.Masks[TBoard::MT_WBISHOP]) * 3
			 + BCNT(Board.Masks[TBoard::MT_WKNIGHT]) * 3
			 + BCNT(Board.Masks[TBoard::MT_WPAWN]);

	int sumb = BCNT(Board.Masks[TBoard::MT_BQUEEN]) * 9
			 + BCNT(Board.Masks[TBoard::MT_BROOK]) * 5
			 + BCNT(Board.Masks[TBoard::MT_BBISHOP]) * 3
			 + BCNT(Board.Masks[TBoard::MT_BKNIGHT]) * 3
			 + BCNT(Board.Masks[TBoard::MT_BPAWN]);
	
	int sum = (sumw - sumb) * 64 / (sumw + sumb + 1);
	return (Board.Turn & 1) ?  -sum : sum;
}

int THeuristics::ImmutablePiecesFactor() {
	int sum;
	switch (GameStage) {
	case GS_OPENNING:
		sum = -BCNT(Board.Masks[TBoard::MT_WKNIGHT] & TBoard::WhiteKnightStartCells)
				+ BCNT(Board.Masks[TBoard::MT_BKNIGHT] & TBoard::BlackKnightStartCells)
				- BCNT(Board.Masks[TBoard::MT_WBISHOP] & TBoard::WhiteBishopStartCells)
				+ BCNT(Board.Masks[TBoard::MT_BBISHOP] & TBoard::BlackBishopStartCells);
		return (Board.Turn & 1) ? -sum : sum;
	default:
		return 0;
	}
}



int THeuristics::PawnProgressFactor() {
	if (GameStage != GS_ENDING)
		return 0;
	int score = 0;
	if (Board.Masks[TBoard::MT_WPAWN]) {
		score += BCNT(TBoard::MaskLine3 & Board.Masks[TBoard::MT_WPAWN])
			   + BCNT(TBoard::MaskLine4 & Board.Masks[TBoard::MT_WPAWN]) * 3
			   + BCNT(TBoard::MaskLine5 & Board.Masks[TBoard::MT_WPAWN]) * 6
			   + BCNT(TBoard::MaskLine6 & Board.Masks[TBoard::MT_WPAWN]) * 10
			   + BCNT(TBoard::MaskLine7 & Board.Masks[TBoard::MT_WPAWN]) * 15;
	}
	if (Board.Masks[TBoard::MT_BPAWN]) {
		score -= BCNT(TBoard::MaskLine6 & Board.Masks[TBoard::MT_BPAWN])
			   + BCNT(TBoard::MaskLine5 & Board.Masks[TBoard::MT_BPAWN]) * 3
			   + BCNT(TBoard::MaskLine4 & Board.Masks[TBoard::MT_BPAWN]) * 6
			   + BCNT(TBoard::MaskLine3 & Board.Masks[TBoard::MT_BPAWN]) * 10
			   + BCNT(TBoard::MaskLine2 & Board.Masks[TBoard::MT_BPAWN]) * 15;
	}
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::BlockedPawnFactor() {
    auto all = MaskByColor[0] | MaskByColor[1];
	int score =
		BCNT(Board.Masks[TBoard::MT_BPAWN] & (all << 8)) -
		BCNT(Board.Masks[TBoard::MT_WPAWN] & (all >> 8));
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::KnightActivityFactor(int wk, int bk) {
	int score = BCNT(TBoard::KnightByDistArea[bk][0] & Board.Masks[TBoard::MT_WKNIGHT]) * 3
			  + BCNT(TBoard::KnightByDistArea[bk][1] & Board.Masks[TBoard::MT_WKNIGHT]) * 2
			  + BCNT(TBoard::KnightByDistArea[bk][1] & Board.Masks[TBoard::MT_WKNIGHT])
			  - BCNT(TBoard::KnightByDistArea[wk][0] & Board.Masks[TBoard::MT_BKNIGHT]) * 3
			  - BCNT(TBoard::KnightByDistArea[wk][1] & Board.Masks[TBoard::MT_BKNIGHT]) * 2
			  - BCNT(TBoard::KnightByDistArea[wk][1] & Board.Masks[TBoard::MT_BKNIGHT]);
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::BishopActivityFactor(int wk, int bk) {
	int score = BCNT(TBoard::DiagonalMask[bk] & Board.Masks[TBoard::MT_WBISHOP])
	          - BCNT(TBoard::DiagonalMask[wk] & Board.Masks[TBoard::MT_BBISHOP]);
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::KingCentralityFactor(int wk, int bk) {
	if (GameStage != GS_ENDING)
		return 0;
	int score = TBoard::KingCentrality[wk] - TBoard::KingCentrality[bk];
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::DoubleRookFactor() {
	
}

const vector<int> THeuristics::DefaultUseFactors = { 9, 1, 1, 2, 2, 1, 1, 1, 1 };
