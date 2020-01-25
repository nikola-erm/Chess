#include "board.h"

#include <util.h>

#include <cstring>
#include <iostream>
#include <queue>

using namespace NBoard;

bool TBoard::StaticInitDone = false;

void TBoard::StaticInit() {
    if (StaticInitDone)
        return;
    StaticInitDone = true;
	for (int m = 0; m < 256; m++) {
		int wi = 0;
		for (int j = 0; j < 8; j++)
			if (m & (1 << j))
				Bits[m][wi++] = j;
		Bits[m][wi] = -1;
	}
	for (int x = 0; x < 8; x++)
	for (int y = 0; y < 8; y++) {
		int from = Num(x, y);
		{
			KingStepAll[from] = 0;
			int wi = 0;
			for (int dx : { -1, 0, 1 })
			for (int dy : { -1, 0, 1 }) {
				if (dx == 0 && dy == 0)
					continue;
				int nx = x + dx, ny = y + dy;
				if (Valid(nx, ny)) {
					KingStep[from][wi++] = Num(nx, ny);
					KingStepAll[from] |= GetMask(nx, ny);
				}	
			}
			KingStep[from][wi] = -1;
		}
		{
			KnightStepAll[from] = 0;
			int wi = 0;
			int dx[] = { 2, 1, -1, -2, -2, -1, 1, 2 };
			int dy[] = { 1, 2, 2, 1, -1, -2, -2, -1 };
			for (int dir = 0; dir < 8; dir++) {
				int nx = x + dx[dir];
				int ny = y + dy[dir];
				if (!Valid(nx, ny))
					continue;
				KnightStep[from][wi++] = Num(nx, ny);
				KnightStepAll[from] |= GetMask(nx, ny);
			}
			KnightStep[from][wi] = -1;
			for (int d = 0; d < 3; d++)
		    	KnightByDistArea[from][d] = 0;
		    static int was[8][8];
		    memset(was, 0xff, sizeof(was));
		    was[x][y] = 0;
		    queue<pair<int, int>> q;
		    q.push({x, y});
		    while (!q.empty()) {
		    	int x = q.front().first;
		    	int y = q.front().second;
		    	q.pop();
		    	if (was[x][y] >= 3)
		    		continue;
		    	for (int dir = 0; dir < 8; dir++) {
					int nx = x + dx[dir];
					int ny = y + dy[dir];
					if (!Valid(nx, ny) || was[nx][ny] != -1)
						continue;
					was[nx][ny] = was[x][y] + 1;
					KnightByDistArea[from][was[nx][ny] - 1] |= GetMask(nx, ny);
					q.push({nx, ny});
				}	
		    }
		}
		{
			int dx[] = { 1, 0, -1, 0 };
			int dy[] = { 0, 1, 0, -1 };
			for (int dir = 0; dir < 4; dir++) {
				RookStepAll[from][dir] = 0;
				int i = 0;
				int nx = x + dx[dir];
				int ny = y + dy[dir];
				while (Valid(nx, ny)) {
					RookStepAll[from][dir] |= GetMask(RookStep[from][dir][i] = Num(nx, ny));
					i++;
					nx += dx[dir];
					ny += dy[dir];
				}
				RookStep[from][dir][i] = -1;
			}
		}
		{
			DiagonalMask[from] = 0;
			int dx[] = { 1, -1, -1, 1 };
			int dy[] = { 1, 1, -1, -1 };
			for (int dir = 0; dir < 4; dir++) {
				BishopStepAll[from][dir] = 0;
				int i = 0;
				int nx = x + dx[dir];
				int ny = y + dy[dir];
				while (Valid(nx, ny)) {
					BishopStepAll[from][dir] |= GetMask(BishopStep[from][dir][i] = Num(nx, ny));
					i++;
					nx += dx[dir];
					ny += dy[dir];
				}
				BishopStep[from][dir][i] = -1;
				DiagonalMask[from] |= BishopStepAll[from][dir];
			}
		}
        {
			DiagonalMask[from] = 0;
			int dx[] = { 1, -1, -1, 1 };
			int dy[] = { 1, 1, -1, -1 };
			for (int dir = 0; dir < 4; dir++) {
				BishopStepAll[from][dir] = 0;
				int i = 0;
				int nx = x + dx[dir];
				int ny = y + dy[dir];
				while (Valid(nx, ny)) {
					BishopStepAll[from][dir] |= GetMask(BishopStep[from][dir][i] = Num(nx, ny));
					i++;
					nx += dx[dir];
					ny += dy[dir];
				}
				BishopStep[from][dir][i] = -1;
				DiagonalMask[from] |= BishopStepAll[from][dir];
			}
		}
		{
			auto pos = from;
			if (y < 7)
				PawnWhiteStep[pos][0] = Num(x, y + 1);
			if (y > 0)
				PawnBlackStep[pos][0] = Num(x, y - 1);
			PawnWhiteStep[pos][1] = (y == 1 ? Num(x, y + 2) : -1);
			PawnBlackStep[pos][1] = (y == 6 ? Num(x, y - 2) : -1);
			PawnWhiteStrikeAll[pos] = 0;
			PawnBlackStrikeAll[pos] = 0;
			int wi = 0;
			int bi = 0;
			for (int dx : { -1, 1 }) {
				if (Valid(x + dx, y + 1))
					PawnWhiteStrikeAll[pos] |= GetMask(PawnWhiteStrike[pos][wi++] = Num(x + dx, y + 1));
				if (Valid(x + dx, y - 1))
					PawnBlackStrikeAll[pos] |= GetMask(PawnBlackStrike[pos][bi++] = Num(x + dx, y - 1));
			}
			PawnWhiteStrike[pos][wi] = -1;
			PawnBlackStrike[pos][bi] = -1;
		}

		RequiredEnPassantMask[from][0] = 0;
		RequiredEnPassantMask[from][1] = 0;
		if (y == 3) {
		  	if (x > 0) {
				RequiredEnPassantMask[from][0] = GetMask(x-1, 1) | GetMask(x-1, 3);
				auto& m = EnPassantMove[from][0];
				m.Reset();
				m.Add(MT_BPAWN, from, Num(x-1, 2));
				m.Add(MT_WPAWN, Num(x-1, 3));
			}
			if (x < 7) {
				RequiredEnPassantMask[from][1] = GetMask(x+1, 1) | GetMask(x+1, 3);
				auto& m = EnPassantMove[from][1];
				m.Reset();
				m.Add(MT_BPAWN, from, Num(x+1, 2));
				m.Add(MT_WPAWN, Num(x+1, 3));
			}
		} else if (y == 4) {
		    if (x > 0) {
		    	RequiredEnPassantMask[from][0] = GetMask(x-1, 6) | GetMask(x-1, 4);
		    	auto& m = EnPassantMove[from][0];
				m.Reset();
				m.Add(MT_WPAWN, from, Num(x-1, 5));
				m.Add(MT_BPAWN, Num(x-1, 4));
		    }
		    if (x < 7) {
		    	RequiredEnPassantMask[from][1] = GetMask(x+1, 6) | GetMask(x+1, 4);
		    	auto& m = EnPassantMove[from][1];
				m.Reset();
				m.Add(MT_WPAWN, from, Num(x+1, 5));
				m.Add(MT_BPAWN, Num(x+1, 4));
		    }
		    
		}

        KingCentrality[from] = min(x + 1, 8 - x) * min(y + 1, 8 - y) - 1;

		FieldStr[from] = "";
		FieldStr[from] += (char)('a' + x);
		FieldStr[from] += (char)('1' + y);
	}
	WhiteKnightStartCells = GetMask(1, 0) | GetMask(6, 0);
	BlackKnightStartCells = GetMask(1, 7) | GetMask(6, 7);
	WhiteBishopStartCells = GetMask(2, 0) | GetMask(5, 0);
	BlackBishopStartCells = GetMask(2, 7) | GetMask(5, 7);
	PawnTransformLines = 0;
	for (int i = 0; i < 8; i++)
		PawnTransformLines |= GetMask(i, 0) | GetMask(i, 7);
	MaskLine1 = 0xffull;
	MaskLine2 = 0xff00ull;
	MaskLine3 = 0xff0000ull;
	MaskLine4 = 0xff000000ull;
	MaskLine5 = 0xff00000000ull;
	MaskLine6 = 0xff0000000000ull;
	MaskLine7 = 0xff000000000000ull;
	MaskLine8 = 0xff00000000000000ull;
	MaskColumnA = GetMask(0, 0) | GetMask(0, 1) | GetMask(0, 2) | GetMask(0, 3)
				| GetMask(0, 4) | GetMask(0, 5) | GetMask(0, 6) | GetMask(0, 7);
	MaskColumnB = MaskColumnA << 1;
	MaskColumnC = MaskColumnA << 2;
	MaskColumnD = MaskColumnA << 3;
	MaskColumnE = MaskColumnA << 4;
	MaskColumnF = MaskColumnA << 5;
	MaskColumnG = MaskColumnA << 6;
	MaskColumnH = MaskColumnA << 7;
	for (int mask = 0; mask < 256; mask++) {
		int cnt = 0;
		for (int i = 0; i < 8; i++)
			if (mask & (1 << i)) {
				if ((i == 0 || !(mask & (1 << (i-1))))
					&& (i == 7 || !(mask & (1 << (i+1)))))
						cnt++;
			}
		IsolatedPawnCount[mask] = cnt;
	}
	LongCastlingWhiteCoridor  = GetMask(1, 0) | GetMask(2, 0) | GetMask(3, 0);
	LongCastlingBlackCoridor  = GetMask(1, 7) | GetMask(2, 7) | GetMask(3, 7);
	ShortCastlingWhiteCoridor = GetMask(5, 0) | GetMask(6, 0);
	ShortCastlingBlackCoridor = GetMask(5, 7) | GetMask(6, 7);

	ShortCastlingBlackKingDiff[0] = Num(4, 7);
	ShortCastlingBlackKingDiff[1] = Num(6, 7);
	ShortCastlingWhiteKingDiff[0] = Num(4, 0);
	ShortCastlingWhiteKingDiff[1] = Num(6, 0);
	LongCastlingBlackKingDiff[0] = Num(4, 7);
	LongCastlingBlackKingDiff[1] = Num(2, 7);
	LongCastlingWhiteKingDiff[0] = Num(4, 0);
	LongCastlingWhiteKingDiff[1] = Num(2, 0);
	ShortCastlingBlackRookDiff[0] = Num(7, 7);
	ShortCastlingBlackRookDiff[1] = Num(5, 7);
	ShortCastlingWhiteRookDiff[0] = Num(7, 0);
	ShortCastlingWhiteRookDiff[1] = Num(5, 0);
	LongCastlingBlackRookDiff[0] = Num(0, 7);
	LongCastlingBlackRookDiff[1] = Num(3, 7);
	LongCastlingWhiteRookDiff[0] = Num(0, 0);
	LongCastlingWhiteRookDiff[1] = Num(3, 0);

	for (int i = 0; i < 14; i++)
	for (int j = 0; j < 64; j++) {
		auto& fp = FigurePrints[i][j];
		fp = 0;
		for (int pos = 5; pos < 64; pos++) {
			fp |= static_cast<TMask>(Rand(2)) << pos;
		}
	}
}

int   TBoard::Bits[256][9];
int TBoard::KingStep[64][9];
TMask TBoard::KingStepAll[64];
int TBoard::KnightStep[64][9];
TMask TBoard::KnightStepAll[64];
int TBoard::RookStep[64][4][8];
TMask TBoard::RookStepAll[64][4];
int TBoard::BishopStep[64][4][8];
TMask TBoard::BishopStepAll[64][4];
int TBoard::PawnWhiteStep[64][3];
int TBoard::PawnBlackStep[64][3];
int TBoard::PawnWhiteStrike[64][3];
int TBoard::PawnBlackStrike[64][3];
TMask TBoard::PawnWhiteStrikeAll[64];
TMask TBoard::PawnBlackStrikeAll[64];
TMask TBoard::PawnTransformLines;
string TBoard::FieldStr[64];
TMask TBoard::WhiteKnightStartCells;
TMask TBoard::BlackKnightStartCells;
TMask TBoard::WhiteBishopStartCells;
TMask TBoard::BlackBishopStartCells;
TMask TBoard::MaskLine1;
TMask TBoard::MaskLine2;
TMask TBoard::MaskLine3;
TMask TBoard::MaskLine4;
TMask TBoard::MaskLine5;
TMask TBoard::MaskLine6;
TMask TBoard::MaskLine7;
TMask TBoard::MaskLine8;
TMask TBoard::MaskColumnA;
TMask TBoard::MaskColumnB;
TMask TBoard::MaskColumnC;
TMask TBoard::MaskColumnD;
TMask TBoard::MaskColumnE;
TMask TBoard::MaskColumnF;
TMask TBoard::MaskColumnG;
TMask TBoard::MaskColumnH;
int TBoard::IsolatedPawnCount[256];
TMask TBoard::ShortCastlingWhiteCoridor;
TMask TBoard::ShortCastlingBlackCoridor;
TMask TBoard::LongCastlingWhiteCoridor;
TMask TBoard::LongCastlingBlackCoridor;
int TBoard::ShortCastlingBlackKingDiff[2];
int TBoard::ShortCastlingWhiteKingDiff[2];
int TBoard::LongCastlingBlackKingDiff[2];
int TBoard::LongCastlingWhiteKingDiff[2];
int TBoard::ShortCastlingBlackRookDiff[2];
int TBoard::ShortCastlingWhiteRookDiff[2];
int TBoard::LongCastlingBlackRookDiff[2];
int TBoard::LongCastlingWhiteRookDiff[2];
TMask TBoard::RequiredEnPassantMask[64][2];
TMove TBoard::EnPassantMove[64][2];
TMask TBoard::FigurePrints[14][64];
TMask TBoard::KnightByDistArea[64][3];
TMask TBoard::DiagonalMask[64];
int TBoard::KingCentrality[64];