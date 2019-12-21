#include "heuristics.h"



THeuristics::THeuristics(const NBoard::TBoard& board)
	: Board(board) {}

void THeuristics::UpdateGameStage() {
	if (Board.Turn < 20) {
		GameStage = GS_OPENNING;
	} else {
		for (int i = 0; i < 2; i++)
			MaskByColor[i] = Board.Masks[NBoard::MT_WQUEEN + i]
				| Board.Masks[NBoard::MT_WROOK + i]
				| Board.Masks[NBoard::MT_WBISHOP + i]
				| Board.Masks[NBoard::MT_WKNIGHT + i]
				| Board.Masks[NBoard::MT_WPAWN + i]
				| Board.Masks[NBoard::MT_WKING + i];
		GameStage = BCNT(MaskByColor[0] | MaskByColor[1]) < 16 ? GS_ENDING : GS_MIDDLE;
	}
}

int THeuristics::GetScore() {
	UpdateGameStage();
	int wk = GetBitPos(Board.Masks[NBoard::MT_WKING]);
	int bk = GetBitPos(Board.Masks[NBoard::MT_BKING]);
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
    if (UseFactors[PAWN_RAW_FACTOR])
        score += PawnRawFactor() * UseFactors[PAWN_RAW_FACTOR];
    if (UseFactors[PAWN_STRIKE_FACTOR])
        score += PawnStrikeFactor() * UseFactors[PAWN_STRIKE_FACTOR];
    if (UseFactors[KING_BETWEEN_ROOKS_FACTOR])
        score += KingBetweenRooksFactor(wk, bk) * UseFactors[KING_BETWEEN_ROOKS_FACTOR];
	//if (UseFactors[DOUBLE_ROOK_FACTOR])
	//	score += DoubleRookFactor() * UseFactors[DOUBLE_ROOK_FACTOR];
	return score;
}

int THeuristics::IsolatedPawnFactor() {
	int wm = 0, bm = 0;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnA) wm |= 1;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnA) bm |= 1;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnB) wm |= 2;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnB) bm |= 2;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnC) wm |= 4;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnC) bm |= 4;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnD) wm |= 8;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnD) bm |= 8;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnE) wm |= 16;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnE) bm |= 16;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnF) wm |= 32;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnF) bm |= 32;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnG) wm |= 64;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnG) bm |= 64;
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumnH) wm |= 128;
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumnH) bm |= 128;
	if (Board.Turn & 1)
		return NBoard::TBoard::IsolatedPawnCount[wm] - NBoard::TBoard::IsolatedPawnCount[bm];
	else
		return NBoard::TBoard::IsolatedPawnCount[bm] - NBoard::TBoard::IsolatedPawnCount[wm];
}

int THeuristics::DoubledPawnsFactor() {
	int score = 0;
#define DOUBLE_PAWN_FACTOR_FOR_COLUMN(c) \
	if (Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumn##c) score += 1 - BCNT(Board.Masks[NBoard::MT_WPAWN] & NBoard::TBoard::MaskColumn##c); \
	if (Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumn##c) score -= 1 - BCNT(Board.Masks[NBoard::MT_BPAWN] & NBoard::TBoard::MaskColumn##c);
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
	int sumw = BCNT(Board.Masks[NBoard::MT_WQUEEN]) * 9
			 + BCNT(Board.Masks[NBoard::MT_WROOK]) * 5
			 + BCNT(Board.Masks[NBoard::MT_WBISHOP]) * 3
			 + BCNT(Board.Masks[NBoard::MT_WKNIGHT]) * 3
			 + BCNT(Board.Masks[NBoard::MT_WPAWN]);

	int sumb = BCNT(Board.Masks[NBoard::MT_BQUEEN]) * 9
			 + BCNT(Board.Masks[NBoard::MT_BROOK]) * 5
			 + BCNT(Board.Masks[NBoard::MT_BBISHOP]) * 3
			 + BCNT(Board.Masks[NBoard::MT_BKNIGHT]) * 3
			 + BCNT(Board.Masks[NBoard::MT_BPAWN]);
	
	int sum = (sumw - sumb) * 64 / (sumw + sumb + 1);
	return (Board.Turn & 1) ?  -sum : sum;
}

int THeuristics::ImmutablePiecesFactor() {
	int sum;
	switch (GameStage) {
	case GS_OPENNING:
		sum =   - BCNT(Board.Masks[NBoard::MT_WKNIGHT] & NBoard::TBoard::WhiteKnightStartCells)
				+ BCNT(Board.Masks[NBoard::MT_BKNIGHT] & NBoard::TBoard::BlackKnightStartCells)
				- BCNT(Board.Masks[NBoard::MT_WBISHOP] & NBoard::TBoard::WhiteBishopStartCells)
				+ BCNT(Board.Masks[NBoard::MT_BBISHOP] & NBoard::TBoard::BlackBishopStartCells);
		return (Board.Turn & 1) ? -sum : sum;
	default:
		return 0;
	}
}



int THeuristics::PawnProgressFactor() {
	if (GameStage != GS_ENDING)
		return 0;
	int score = 0;
	if (Board.Masks[NBoard::MT_WPAWN]) {
		score += BCNT(NBoard::TBoard::MaskLine3 & Board.Masks[NBoard::MT_WPAWN])
			   + BCNT(NBoard::TBoard::MaskLine4 & Board.Masks[NBoard::MT_WPAWN]) * 3
			   + BCNT(NBoard::TBoard::MaskLine5 & Board.Masks[NBoard::MT_WPAWN]) * 6
			   + BCNT(NBoard::TBoard::MaskLine6 & Board.Masks[NBoard::MT_WPAWN]) * 10
			   + BCNT(NBoard::TBoard::MaskLine7 & Board.Masks[NBoard::MT_WPAWN]) * 15;
	}
	if (Board.Masks[NBoard::MT_BPAWN]) {
		score -= BCNT(NBoard::TBoard::MaskLine6 & Board.Masks[NBoard::MT_BPAWN])
			   + BCNT(NBoard::TBoard::MaskLine5 & Board.Masks[NBoard::MT_BPAWN]) * 3
			   + BCNT(NBoard::TBoard::MaskLine4 & Board.Masks[NBoard::MT_BPAWN]) * 6
			   + BCNT(NBoard::TBoard::MaskLine3 & Board.Masks[NBoard::MT_BPAWN]) * 10
			   + BCNT(NBoard::TBoard::MaskLine2 & Board.Masks[NBoard::MT_BPAWN]) * 15;
	}
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::BlockedPawnFactor() {
    auto all = MaskByColor[0] | MaskByColor[1];
	int score =
		BCNT(Board.Masks[NBoard::MT_BPAWN] & (all << 8)) -
		BCNT(Board.Masks[NBoard::MT_WPAWN] & (all >> 8));
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::KnightActivityFactor(int wk, int bk) {
	int score = BCNT(NBoard::TBoard::KnightByDistArea[bk][0] & Board.Masks[NBoard::MT_WKNIGHT]) * 3
			  + BCNT(NBoard::TBoard::KnightByDistArea[bk][1] & Board.Masks[NBoard::MT_WKNIGHT]) * 2
			  + BCNT(NBoard::TBoard::KnightByDistArea[bk][1] & Board.Masks[NBoard::MT_WKNIGHT])
			  - BCNT(NBoard::TBoard::KnightByDistArea[wk][0] & Board.Masks[NBoard::MT_BKNIGHT]) * 3
			  - BCNT(NBoard::TBoard::KnightByDistArea[wk][1] & Board.Masks[NBoard::MT_BKNIGHT]) * 2
			  - BCNT(NBoard::TBoard::KnightByDistArea[wk][1] & Board.Masks[NBoard::MT_BKNIGHT]);
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::BishopActivityFactor(int wk, int bk) {
	int score = BCNT(NBoard::TBoard::DiagonalMask[bk] & Board.Masks[NBoard::MT_WBISHOP])
	          - BCNT(NBoard::TBoard::DiagonalMask[wk] & Board.Masks[NBoard::MT_BBISHOP]);
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::KingCentralityFactor(int wk, int bk) {
	if (GameStage != GS_ENDING)
		return 0;
	int score = NBoard::TBoard::KingCentrality[wk] - NBoard::TBoard::KingCentrality[bk];
	return (Board.Turn & 1) ? -score : score;
}

int THeuristics::PawnRawFactor() {
    int score = BCNT((~NBoard::TBoard::MaskColumnA) & Board.Masks[NBoard::MT_WPAWN] & (Board.Masks[NBoard::MT_WPAWN] << 1))
              - BCNT((~NBoard::TBoard::MaskColumnA) & Board.Masks[NBoard::MT_BPAWN] & (Board.Masks[NBoard::MT_BPAWN] << 1));
    return (Board.Turn & 1) ? -score : score;

}

int THeuristics::PawnStrikeFactor() {
    int score = BCNT((((~NBoard::TBoard::MaskColumnA) & Board.Masks[NBoard::MT_WPAWN]) << 7) & (
                    Board.Masks[NBoard::MT_BKING] | Board.Masks[NBoard::MT_BQUEEN] | Board.Masks[NBoard::MT_BROOK] | Board.Masks[NBoard::MT_BBISHOP] | Board.Masks[NBoard::MT_BKNIGHT]
                )) -
                BCNT((((~NBoard::TBoard::MaskColumnH) & Board.Masks[NBoard::MT_BPAWN]) << 9) & (
                    Board.Masks[NBoard::MT_WKING] | Board.Masks[NBoard::MT_WQUEEN] | Board.Masks[NBoard::MT_WROOK] | Board.Masks[NBoard::MT_WBISHOP] | Board.Masks[NBoard::MT_WKNIGHT]
                ));
    return (Board.Turn & 1) ? -score : score;
}

int THeuristics::KingBetweenRooksFactor(int wk, int bk) {
    int score = (int)((Board.Masks[NBoard::MT_BROOK] & NBoard::TBoard::RookStepAll[bk][0]) && (Board.Masks[NBoard::MT_BROOK] & NBoard::TBoard::RookStepAll[bk][2]))
              + (int)((Board.Masks[NBoard::MT_BROOK] & NBoard::TBoard::RookStepAll[bk][1]) && (Board.Masks[NBoard::MT_BROOK] & NBoard::TBoard::RookStepAll[bk][3]))
              - (int)((Board.Masks[NBoard::MT_WROOK] & NBoard::TBoard::RookStepAll[wk][0]) && (Board.Masks[NBoard::MT_WROOK] & NBoard::TBoard::RookStepAll[wk][2]))
              - (int)((Board.Masks[NBoard::MT_WROOK] & NBoard::TBoard::RookStepAll[wk][1]) && (Board.Masks[NBoard::MT_WROOK] & NBoard::TBoard::RookStepAll[wk][3]));
    return (Board.Turn & 1) ? -score : score;
}

int THeuristics::DoubleRookFactor() {
}

//const vector<int> THeuristics::DefaultUseFactors = { 9, 1, 1, 2, 2, 1, 1, 1, 1, 0 };
const vector<int> THeuristics::DefaultUseFactors = { 9, 5, 1, 0, 1, 0, 1, 2, 2, 1, 0, 1 };
