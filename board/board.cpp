#include "board.h"

#include <util.h>

#include <iostream>

#define VBP(pos) \
	if (pos == -1) { \
		cerr << "invalid bit pos at " << __FUNCTION__ << endl; \
		Print(); \
		exit(0); \
	}

TBoard::TBoard() {
	InitStandart();
}

void TBoard::MakeMove(const TMove& m, const string& name) {
	StoryNames[Turn] = name;
	MakeMove(m);
}

void TBoard::MakeMove(const TMove& m) {
	Story[Turn] = m;
	Turn++;
	for (int i = 0; i < m.Count; i++) {
		Masks[m.MaskTypes[i]] ^= m.Masks[i];
	}
	if (m.MaskTypes[0] == MT_WKING) {
		if (Masks[MT_HASH] & 3) {
			cerr << "Earlier fail!" << endl;
			for (int i = 0; i < m.Count; i++) {
				cerr << "type: " << m.MaskTypes[i] << endl;
				cerr << "mask: " << m.Masks[i] << endl;
			}	
			Print();
			exit(0);
		}
	}
	if (BCNT(Masks[MT_WKING]) != 1) {
		for (int i = 0; i < m.Count; i++) {
			cerr << "type: " << m.MaskTypes[i] << endl;
			cerr << "mask: " << m.Masks[i] << endl;
		}	
		Print();
		exit(0);
	}
}

void TBoard::UndoMove(const TMove& m) {
	Turn--;
	for (int i = 0; i < m.Count; i++)
		Masks[m.MaskTypes[i]] ^= m.Masks[i];
}

bool TBoard::IsOpKingUnderAttack() const {
	if (Turn & 1)
		return IsWhiteKingUnderAttack();
	else
		return IsBlackKingUnderAttack();
}

bool TBoard::IsMyKingUnderAttack() const {
	if (Turn & 1)
		return IsBlackKingUnderAttack();
	else
		return IsWhiteKingUnderAttack();
}

bool TBoard::IsUnderLinearAttack(
	int pos, 
	const TMask& rook,
	const TMask& bishop,
	const TMask& queen,
	const TMask& passive
) const
{
	for (int dir = 2; dir < 4; dir++) {
    	if (((rook | queen) & RookStepAll[pos][dir]) > ((bishop | passive) & RookStepAll[pos][dir]))
    		return true;
    	if (((bishop | queen) & BishopStepAll[pos][dir]) > ((rook | passive) & BishopStepAll[pos][dir]))
    		return true;
    }
    for (int dir = 0; dir < 2; dir++) {
        TMask safe;
        TMask dangerous = ((rook | queen) & RookStepAll[pos][dir]);
        if (dangerous) {
       		safe = ((bishop | passive) & RookStepAll[pos][dir]);
       		if (safe == 0 || LowestBit(dangerous) < LowestBit(safe))
       			return true; 	
        }
        dangerous = ((bishop | queen) & BishopStepAll[pos][dir]);
	        if (dangerous) {
       		safe = ((rook | passive) & BishopStepAll[pos][dir]);
       		if (safe == 0 || LowestBit(dangerous) < LowestBit(safe))
       			return true; 	
        }
    }
    return false;
}

bool TBoard::IsWhiteKingUnderAttack() const {
	int pos = GetBitPos(Masks[MT_WKING]);
	VBP(pos)
	return IsWhiteUnderAttack(pos);
}
	
bool TBoard::IsWhiteUnderAttack(int pos) const {
	if ((KingStepAll[pos] & Masks[MT_BKING]) ||
		(KnightStepAll[pos] & Masks[MT_BKNIGHT]) ||
		(PawnWhiteStrikeAll[pos] & Masks[MT_BPAWN]))
			return true;
	return IsUnderLinearAttack(
			pos,
			Masks[MT_BROOK],
			Masks[MT_BBISHOP],
			Masks[MT_BQUEEN],
			Masks[MT_WKNIGHT] | Masks[MT_WROOK] | Masks[MT_WBISHOP] | Masks[MT_WQUEEN] | Masks[MT_WPAWN] |
			Masks[MT_BKNIGHT] | Masks[MT_BKING] | Masks[MT_BPAWN]);
}

bool TBoard::IsBlackKingUnderAttack() const {
	int pos = GetBitPos(Masks[MT_BKING]);
	VBP(pos)
	return IsBlackUnderAttack(pos);
}

bool TBoard::IsBlackUnderAttack(int pos) const {
	if ((KingStepAll[pos] & Masks[MT_WKING]) ||
		(KnightStepAll[pos] & Masks[MT_WKNIGHT]) ||
		(PawnBlackStrikeAll[pos] & Masks[MT_WPAWN]))
		return true;
	return IsUnderLinearAttack(
		pos,
		Masks[MT_WROOK],
		Masks[MT_WBISHOP],
		Masks[MT_WQUEEN],
		Masks[MT_BKNIGHT] | Masks[MT_BROOK] | Masks[MT_BBISHOP] | Masks[MT_BQUEEN] | Masks[MT_BPAWN] |
		Masks[MT_WKNIGHT] | Masks[MT_WKING] | Masks[MT_WPAWN]);
}

bool TBoard::IsUnderOpAttack(int pos) const {
	if (Turn & 1)
		return IsBlackUnderAttack(pos);
	else
		return IsWhiteUnderAttack(pos);
}

void TBoard::TMove::Reset() {
	Count = 0;
}

void TBoard::TMove::Add(EMaskType maskType, int pos) {
	MaskTypes[Count] = maskType;
	Masks[Count++] = GetMask(pos);
}

void TBoard::TMove::Add(EMaskType maskType, int pos1, int pos2) {
	MaskTypes[Count] = maskType;
	Masks[Count++] = (GetMask(pos1) | GetMask(pos2));
}

void TBoard::TMove::AddHash(TMask mask) {
	MaskTypes[Count] = MT_HASH;
	Masks[Count++] = mask;
}


void TBoard::Print() const {
	cout << (Turn / 2 + 1) << ". ";
	if (Turn & 1)
		cout << "Blacks turn" << endl;
	else
		cout << "Whites turn" << endl;
	string s[8];
	for (int i = 0; i < 8; i++)
		s[i] = "........";
	for (int x = 0; x < 8; x++)
	for (int y = 0; y < 8; y++) {
		auto m = GetMask(x, y);
		if (m & Masks[MT_WPAWN])
			s[y][x] = 'P';
		else if (m & Masks[MT_BPAWN])
			s[y][x] = 'p';
		else if (m & Masks[MT_WKING])
			s[y][x] = 'K';
		else if (m & Masks[MT_BKING])
			s[y][x] = 'k';
		else if (m & Masks[MT_WKNIGHT])
			s[y][x] = 'N';
		else if (m & Masks[MT_BKNIGHT])
			s[y][x] = 'n';
		else if (m & Masks[MT_WROOK])
			s[y][x] = 'R';
		else if (m & Masks[MT_BROOK])
			s[y][x] = 'r';
		else if (m & Masks[MT_WBISHOP])
			s[y][x] = 'B';
		else if (m & Masks[MT_BBISHOP])
			s[y][x] = 'b';
		else if (m & Masks[MT_WQUEEN])
			s[y][x] = 'Q';
		else if (m & Masks[MT_BQUEEN])
			s[y][x] = 'q';									 
	}
	for (int i = 7; i >= 0; i--)
		cout << s[i] << endl;
	cout << endl;
}

void TBoard::PrintStory() const {
	for (int t = 0; t < Turn; t++) {
		if (t % 2 == 0)
			cout << "\n" << t / 2 + 1 << ".";
		cout << " " << StoryNames[t];
	}
}

bool TBoard::TMove::IsCapturing() const {
	bool hw = false, hb = false;
	for (int i = 0; i < Count; i++) {
		switch (MaskTypes[i]) {
		case MT_WKING:
		case MT_WQUEEN:
		case MT_WROOK:
		case MT_WBISHOP:
		case MT_WKNIGHT:
		case MT_WPAWN:
			hw = true;
			break;
		case MT_BKING:
		case MT_BQUEEN:
		case MT_BROOK:
		case MT_BBISHOP:
		case MT_BKNIGHT:
		case MT_BPAWN:
			hb = true;
			break;
		default:
			break;			
		}
	}
	return hw && hb;
}
