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
}