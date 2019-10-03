#include "heuristics.h"



THeuristics::THeuristics(const TBoard& board)
	: Board(board) {}

void THeuristics::UpdateGameStage() {
	if (Board.Turn < 14) {
		GameStage = GS_OPENNING;
	} else {
		TMask m = Board.Masks[TBoard::MT_WQUEEN]  | Board.Masks[TBoard::MT_BQUEEN]
				| Board.Masks[TBoard::MT_WROOK]   | Board.Masks[TBoard::MT_BROOK]
				| Board.Masks[TBoard::MT_WBISHOP] | Board.Masks[TBoard::MT_BBISHOP]
				| Board.Masks[TBoard::MT_WKNIGHT] | Board.Masks[TBoard::MT_BKNIGHT]
				| Board.Masks[TBoard::MT_WPAWN]   | Board.Masks[TBoard::MT_BPAWN];
		GameStage = BCNT(m) < 14 ? GS_ENDING : GS_MIDDLE;
	}
}

int THeuristics::GetScore() {
	UpdateGameStage();
	int score = 0;
	score += MaterialFactor() << 3;
	score += ImmutablePiecesFactor();
	score += PawnProgressFactor();
	score += DoubledPawnsFactor() << 1;
	score += IsolatedPawnFactor() << 1;
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
	if (Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumn##c) score += 1 - BCNT(Board.Masks[TBoard::MT_BPAWN] & TBoard::MaskColumn##c);
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
	
	int sum = (sumw - sumb) * 64 / (sumw + sumb);
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