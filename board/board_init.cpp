#include "board.h"

#include<util.h>

void TBoard::InitStandart() {
	Turn = 0;
	Masks[MT_WKING]   = GetMask(4, 0);
	Masks[MT_BKING]   = GetMask(4, 7);
	Masks[MT_WKNIGHT] = GetMask(1, 0) | GetMask(6, 0);
	Masks[MT_BKNIGHT] = GetMask(1, 7) | GetMask(6, 7);
	Masks[MT_WROOK]   = GetMask(0, 0) | GetMask(7, 0);
	Masks[MT_BROOK]   = GetMask(0, 7) | GetMask(7, 7);
	Masks[MT_WBISHOP] = GetMask(2, 0) | GetMask(5, 0);
	Masks[MT_BBISHOP] = GetMask(2, 7) | GetMask(5, 7);
	Masks[MT_WQUEEN]  = GetMask(3, 0);
	Masks[MT_BQUEEN]  = GetMask(3, 7);
	Masks[MT_WPAWN] = Masks[MT_BPAWN] = 0;
	for (int x = 0; x < 8; x++) {
		Masks[MT_WPAWN] |= GetMask(x, 1);
		Masks[MT_BPAWN] |= GetMask(x, 6);
	}
	Masks[MT_HASH] = 15;
	WasCount[15] = 1;
	Status = GS_PLAY;
}

void TBoard::InitFromFEN(const string& fen) {
	for (int mt = 0; mt < 14; mt++)
		Masks[mt] = 0;
	int i = 0, y = 7, x = 0;
	while (fen[i] != ' ') {
		if (fen[i] == '/') {
			y--;
			x = 0;
			i++;
			continue;
		}
		if ('0' <= fen[i] && fen[i] <= '9') {
			x += fen[i] - '0';
			i++;
			continue;
		}
		switch (fen[i]) {
		case 'K':
			Masks[MT_WKING] |= GetMask(x, y);
			break;
		case 'k':
			Masks[MT_BKING] |= GetMask(x, y);
			break;
		case 'Q':
			Masks[MT_WQUEEN] |= GetMask(x, y);
			break;
		case 'q':
			Masks[MT_BQUEEN] |= GetMask(x, y);
			break;
		case 'R':
			Masks[MT_WROOK] |= GetMask(x, y);
			break;
		case 'r':
			Masks[MT_BROOK] |= GetMask(x, y);
			break;
		case 'B':
			Masks[MT_WBISHOP] |= GetMask(x, y);
			break;
		case 'b':
			Masks[MT_BBISHOP] |= GetMask(x, y);
			break;
		case 'N':
			Masks[MT_WKNIGHT] |= GetMask(x, y);
			break;
		case 'n':
			Masks[MT_BKNIGHT] |= GetMask(x, y);
			break;
		case 'P':
			Masks[MT_WPAWN] |= GetMask(x, y);
			break;
		case 'p':
			Masks[MT_BPAWN] |= GetMask(x, y);
			break;
		}
		x++;
		i++;
	}
	i++; //skip space
	if (fen[i] == 'w') {
	    Turn = 0;
	} else {
		assert(fen[i] == 'b');
		Turn = 1;
	}
	i += 2;
	while (fen[i] != ' ') {
		switch (fen[i]) {
		case 'K':
			Masks[MT_HASH] |= 1;
			break;
		case 'Q':
			Masks[MT_HASH] |= 2;
			break;
		case 'k':
			Masks[MT_HASH] |= 4;
			break;
		case 'q':
			Masks[MT_HASH] |= 8;
			break;
		}
		i++;
	}
	WasCount[Masks[MT_HASH]] = 1;
	Status = GS_PLAY;
	//ignore counters
}