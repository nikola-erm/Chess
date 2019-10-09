#include "board.h"

#include <util.h>

#include <iostream>

TBoard::TMove* TBoard::GenerateMovesUnchecked(TMove* moves) {
	auto c = Turn & 1;

	const auto mtMyKing = (c ? MT_BKING : MT_WKING);
	const auto mtOpKing = (c ? MT_WKING : MT_BKING);
	const auto mtMyKnight = (c ? MT_BKNIGHT : MT_WKNIGHT);
	const auto mtOpKnight = (c ? MT_WKNIGHT : MT_BKNIGHT);
	const auto mtMyRook   = (c ? MT_BROOK : MT_WROOK);
	const auto mtOpRook   = (c ? MT_WROOK : MT_BROOK);
	const auto mtMyBishop = (c ? MT_BBISHOP : MT_WBISHOP);
	const auto mtOpBishop = (c ? MT_WBISHOP : MT_BBISHOP);
	const auto mtMyQueen  = (c ? MT_BQUEEN : MT_WQUEEN);
	const auto mtOpQueen  = (c ? MT_WQUEEN : MT_BQUEEN);
	const auto mtMyPawn   = (c ? MT_BPAWN : MT_WPAWN);
	const auto mtOpPawn   = (c ? MT_WPAWN : MT_BPAWN);

	const auto& opKing = Masks[mtOpKing];
	const auto& myKing = Masks[mtMyKing];
	const auto& myKnight = Masks[mtMyKnight];
	const auto& opKnight = Masks[mtOpKnight];
	const auto& myRook = Masks[mtMyRook];
	const auto& opRook = Masks[mtOpRook];
	const auto& myBishop = Masks[mtMyBishop];
	const auto& opBishop = Masks[mtOpBishop];
	const auto& myQueen = Masks[mtMyQueen];
	const auto& opQueen = Masks[mtOpQueen];
	const auto& myPawn = Masks[mtMyPawn];
	const auto& opPawn = Masks[mtOpPawn];

	const auto myColor = myKing | myKnight | myRook | myBishop | myQueen | myPawn;
	const auto opColor = opKing | opKnight | opRook | opBishop | opQueen | opPawn;
	const auto empty = ~(myColor | opColor);

	const auto& pawnStep = (c ? PawnBlackStep : PawnWhiteStep);
	const auto& pawnStrike = (c ? PawnBlackStrike : PawnWhiteStrike);

	auto myKingTmp = myKing;
	auto myKnightTmp = myKnight;
	auto myRookTmp = myRook;
	auto myBishopTmp = myBishop;
	auto myQueenTmp = myQueen;
	auto myPawnTmp = myPawn;
	
	TMask shortCastlingMask = 1;
	TMask longCastlingMask = 2;
	const auto& shortCastlingCoridor = c ? ShortCastlingBlackCoridor : ShortCastlingWhiteCoridor;
	const auto& longCastlingCoridor  = c ? LongCastlingBlackCoridor  : LongCastlingWhiteCoridor;
	int shortCastlingTransit = 5;
	int longCastlingTransit = 3;
	int shortRookStartPos = 7;
	int longRookStartPos = 0;
	const auto& shortCastlingKingDiff = c ? ShortCastlingBlackKingDiff : ShortCastlingWhiteKingDiff;
	const auto& longCastlingKingDiff =  c ? LongCastlingBlackKingDiff : LongCastlingWhiteKingDiff;
	const auto& shortCastlingRookDiff = c ? ShortCastlingBlackRookDiff : ShortCastlingWhiteRookDiff;
	const auto& longCastlingRookDiff =  c ? LongCastlingBlackRookDiff : LongCastlingWhiteRookDiff;

	if (c) {
		shortCastlingMask = 4;
		longCastlingMask = 8;
		shortCastlingTransit += 56;
		longCastlingTransit += 56;
		shortRookStartPos += 56;
		longRookStartPos += 56;
	}
	
	for (int posDelta = 0; posDelta < 64; posDelta += 8) {
		//King moves
		for (auto it = Bits[myKingTmp & 255]; *it != -1; it++) {
			int pos = posDelta + *it;
			auto castlingDiff = (shortCastlingMask | longCastlingMask) & Masks[MT_HASH];
			for (auto m = KingStep[pos]; *m != -1; m++) {
				auto toMask = GetMask(*m);
				if (myColor & toMask) continue;
				moves->Reset();
				moves->Add(mtMyKing, pos, *m);
				auto cap = opColor & toMask;
				if (cap) {
					if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else
						assert(false);	
				}
				if (castlingDiff)
					moves->AddHash(castlingDiff);
				moves++;
			}
			if (castlingDiff && !IsMyKingUnderAttack()) {
				if (
					(shortCastlingMask & Masks[MT_HASH]) &&
					(myRook & (c ? (1ull << 63) : (1ull << 7))) &&
					shortCastlingCoridor == (shortCastlingCoridor & empty) &&
					!IsUnderOpAttack(shortCastlingTransit)
				) {
					moves->Reset();
					moves->Add(mtMyKing, shortCastlingKingDiff[0], shortCastlingKingDiff[1]);
					moves->Add(mtMyRook, shortCastlingRookDiff[0], shortCastlingRookDiff[1]);
					moves->AddHash(castlingDiff);
					moves++;
				}
				if (
					(longCastlingMask & Masks[MT_HASH]) &&
					(myRook & (c ? (1ull << 56) : 1ull)) &&
					longCastlingCoridor == (longCastlingCoridor & empty) &&
					!IsUnderOpAttack(longCastlingTransit)
				) {
					moves->Reset();
					moves->Add(mtMyKing, longCastlingKingDiff[0], longCastlingKingDiff[1]);
					moves->Add(mtMyRook, longCastlingRookDiff[0], longCastlingRookDiff[1]);
					moves->AddHash(castlingDiff);
					moves++;
				} 
			}
		}
		myKingTmp >>= 8;
		//Knight moves
		for (auto it = Bits[myKnightTmp & 255]; *it != -1; it++) {
			int pos = posDelta + *it;
			for (auto m = KnightStep[pos]; *m != -1; m++) {
				auto toMask = GetMask(*m);
				if (myColor & toMask) continue;
				moves->Reset();
				moves->Add(mtMyKnight, pos, *m);
				auto cap = opColor & toMask;
				if (cap) {
					if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else {
						Print();
						PRINT_MASK_IMPL(KnightStepAll[Num(4, 6)])
						PRINT_MASK_IMPL(myKnight)
						assert(false);
					}
				}
				moves++;
			}
		}
		myKnightTmp >>= 8;
		//Rook moves
		for (auto it = Bits[myRookTmp & 255]; *it != -1; it++) {
			int pos = posDelta + *it;
			TMask castlingDiff = 0;
			if (pos == shortRookStartPos)
				castlingDiff = (shortCastlingMask & Masks[MT_HASH]);
			else if (pos == longRookStartPos)
				castlingDiff = (longCastlingMask & Masks[MT_HASH]);
			for (int dir = 0; dir < 4; dir++)
			for (auto m = RookStep[pos][dir]; *m != -1; m++) {
				auto toMask = GetMask(*m);
				if (myColor & toMask) break;
				moves->Reset();
				moves->Add(mtMyRook, pos, *m);
				if (castlingDiff)
					moves->AddHash(castlingDiff);
				auto cap = opColor & toMask;
				if (cap) {
					if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else {
						//Print();
						assert(false);
					}
					moves++;
					break;
				} else {
					moves++;
				}
			}
		}
		myRookTmp >>= 8;
		//Bishop moves
		for (auto it = Bits[myBishopTmp & 255]; *it != -1; it++) {
			int pos = posDelta + *it;
			for (int dir = 0; dir < 4; dir++)
			for (auto m = BishopStep[pos][dir]; *m != -1; m++) {
				auto toMask = GetMask(*m);
				if (myColor & toMask) break;
				moves->Reset();
				moves->Add(mtMyBishop, pos, *m);
				auto cap = opColor & toMask;
				if (cap) {
					if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else {
						for (int t = 0; t < Turn; t++) {
							cerr << StoryNames[t] << endl;
							Story[t].Print();
						}
						Print();
						cerr << "pos = " << pos << endl;
						assert(false);
					}
					moves++;
					break;
				} else {
					moves++;
				}
			}
		}
		myBishopTmp >>= 8;
		//Queen moves
		for (auto it = Bits[myQueenTmp & 255]; *it != -1; it++) {
			int pos = posDelta + *it;
			for (int dir = 0; dir < 4; dir++) {
			for (auto m = RookStep[pos][dir]; *m != -1; m++) {
				auto toMask = GetMask(*m);
				if (myColor & toMask) break;
				moves->Reset();
				moves->Add(mtMyQueen, pos, *m);
				auto cap = opColor & toMask;
				if (cap) {
					if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else {
						assert(false);
					}
					moves++;
					break;
				} else {
					moves++;
				}
			}
			for (auto m = BishopStep[pos][dir]; *m != -1; m++) {
				auto toMask = GetMask(*m);
				if (myColor & toMask) break;
				moves->Reset();
				moves->Add(mtMyQueen, pos, *m);
				auto cap = opColor & toMask;
				if (cap) {
					if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else {
						cerr << "from: " << FieldStr[pos] << endl;
						cerr << "to  : " << FieldStr[GetBitPos(cap)] << endl;
						cerr << "opKing pos: " << FieldStr[GetBitPos(opKing)] << endl;
						assert(false);
					}
					moves++;
					break;
				} else {
					moves++;
				}
			}
			} //for dir
		}
		myQueenTmp >>= 8;
		//Pawn moves
		for (auto it = Bits[myPawnTmp & 255]; *it != -1; it++) {
			int posFrom = posDelta + *it;
			int posTo = pawnStep[posFrom][0];
			auto maskTo = GetMask(posTo);
			if (maskTo & empty) {
				moves->Reset();
				if (maskTo & PawnTransformLines) {
					moves->Add(mtMyPawn, posFrom);	
					moves->Add(mtMyQueen, posTo);
					moves++;

					moves->Reset();
					moves->Add(mtMyPawn, posFrom);
					moves->Add(mtMyRook, posTo);
					moves++;

					moves->Reset();
					moves->Add(mtMyPawn, posFrom);
					moves->Add(mtMyBishop, posTo);
					moves++;

					moves->Reset();
					moves->Add(mtMyPawn, posFrom);
					moves->Add(mtMyKnight, posTo);
					moves++;
				} else {
					moves->Add(mtMyPawn, posFrom, posTo);
					moves++;

					if (-1 != (posTo = pawnStep[posFrom][1])) {
						maskTo = GetMask(posTo);
						if (maskTo & empty) {
							moves->Reset();
							moves->Add(mtMyPawn, posFrom, posTo);
							moves++;
						}
					}
				}
			}
			for (auto m = pawnStrike[posFrom]; *m != -1; m++) {
				auto maskTo = GetMask(*m);
				if (maskTo & opColor) {
					moves->Reset();
					moves->Add(mtMyPawn, posFrom, *m);
					auto cap = opColor & maskTo;
					if (cap & opPawn)
						moves->Add(mtOpPawn, *m);
					else if (cap & opKnight)
						moves->Add(mtOpKnight, *m);
					else if (cap & opBishop)
						moves->Add(mtOpBishop, *m);
					else if (cap & opRook)
						moves->Add(mtOpRook, *m);
					else if (cap & opQueen)
						moves->Add(mtOpQueen, *m);
					else {
						Print();
						assert(false);
					}
					if (maskTo & PawnTransformLines) {
						auto mt2 = moves->MaskTypes[1];	
						moves->Reset();
						moves->Add(mtMyPawn, posFrom);
						moves->Add(mt2, *m);
						moves->Add(mtMyQueen, *m);
						moves++;
					
						moves->Reset();
						moves->Add(mtMyPawn, posFrom);
						moves->Add(mt2, *m);
						moves->Add(mtMyRook, *m);
						moves++;
					
						moves->Reset();
						moves->Add(mtMyPawn, posFrom);
						moves->Add(mt2, *m);
						moves->Add(mtMyBishop, *m);
						moves++;
					
						moves->Reset();
						moves->Add(mtMyPawn, posFrom);
						moves->Add(mt2, *m);
						moves->Add(mtMyKnight, *m);
						moves++;
					} else {
						moves++;
					}
				}           	
			}
			//En-passant
			if (Turn > 0 && Story[Turn-1].MaskTypes[0] == mtOpPawn) {
				const auto& actualMask = Story[Turn-1].Masks[0];
				for (int i = 0; i < 2; i++) {
					if (actualMask == RequiredEnPassantMask[posFrom][i]) {
						*moves = EnPassantMove[posFrom][i];
						moves++;
					}
				}
			}	
		}
		myPawnTmp >>= 8;
	}
	return moves;
}