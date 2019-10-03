#pragma once

#include <config.h>

#include <vector>
#include <string>

using namespace std;

using TMask = unsigned long long;

class TBoard {
public:
	friend class THeuristics;
	friend class TMoveSerializer;

	enum EMaskType {
		MT_NONE,
		MT_HASH,
		MT_WKING,
		MT_BKING,
		MT_WKNIGHT,
		MT_BKNIGHT,
		MT_WROOK,
		MT_BROOK,
		MT_WBISHOP,
		MT_BBISHOP,
		MT_WQUEEN,
		MT_BQUEEN,
		MT_WPAWN,
		MT_BPAWN
	};

	struct TMove {
		EMaskType MaskTypes[5];
		TMask Masks[5];
		int Count;

		void Reset();
		void Add(EMaskType maskType, int pos);
		void Add(EMaskType maskType, int pos1, int pos2);
		void AddHash(TMask mask);
		bool IsCapturing() const;
	};

public:
	static void StaticInit();

	TBoard();

	void MakeMove(const TMove& m, const string& name);
	void MakeMove(const TMove& m);
	void UndoMove(const TMove& m);

	TMove* GenerateMovesUnchecked(TMove* moves);
	
	bool IsOpKingUnderAttack() const;
	bool IsMyKingUnderAttack() const;
	bool IsUnderOpAttack(int pos) const;

	void Print() const;
	void PrintStory() const;
	
private:
	int Turn;
	TMask Masks[14];
	TMove Story[10000];
	string StoryNames[10000];

	bool IsUnderLinearAttack(
		int pos, 
		const TMask& rook,
		const TMask& bishop,
		const TMask& queen,
		const TMask& passive
	) const;
	bool IsWhiteKingUnderAttack() const;
	bool IsBlackKingUnderAttack() const;
	bool IsWhiteUnderAttack(int pos) const;
	bool IsBlackUnderAttack(int pos) const;
	void InitStandart();

	static int Bits[256][9];
	static int KingStep[64][9];
	static TMask KingStepAll[64];
	static int KnightStep[64][9];
	static TMask KnightStepAll[64];
	static int RookStep[64][4][8];
	static TMask RookStepAll[64][4];
	static int BishopStep[64][4][8];
	static TMask BishopStepAll[64][4];
	static int PawnWhiteStep[64][3];
	static int PawnBlackStep[64][3];
	static int PawnWhiteStrike[64][3];
	static int PawnBlackStrike[64][3];
	static TMask PawnWhiteStrikeAll[64];
	static TMask PawnBlackStrikeAll[64];
	static TMask PawnTransformLines;
	static TMask WhiteKnightStartCells;
	static TMask BlackKnightStartCells;
	static TMask WhiteBishopStartCells;
	static TMask BlackBishopStartCells;
	static TMask EnPassantDiff[64];
	static string FieldStr[64];
	static TMask MaskLine1;
	static TMask MaskLine2;
	static TMask MaskLine3;
	static TMask MaskLine4;
	static TMask MaskLine5;
	static TMask MaskLine6;
	static TMask MaskLine7;
	static TMask MaskLine8;
	static TMask MaskColumnA;
	static TMask MaskColumnB;
	static TMask MaskColumnC;
	static TMask MaskColumnD;
	static TMask MaskColumnE;
	static TMask MaskColumnF;
	static TMask MaskColumnG;
	static TMask MaskColumnH;
	static int IsolatedPawnCount[256];
	static TMask ShortCastlingWhiteCoridor;
	static TMask ShortCastlingBlackCoridor;
	static TMask LongCastlingWhiteCoridor;
	static TMask LongCastlingBlackCoridor;
	static int ShortCastlingBlackKingDiff[2];
	static int ShortCastlingWhiteKingDiff[2];
	static int LongCastlingBlackKingDiff[2];
	static int LongCastlingWhiteKingDiff[2];
	static int ShortCastlingBlackRookDiff[2];
	static int ShortCastlingWhiteRookDiff[2];
	static int LongCastlingBlackRookDiff[2];
	static int LongCastlingWhiteRookDiff[2];
};

class TBoardBatch {
public:
	TBoard& operator[](int i);
	TBoard::TMove* GenerateMovesUnchecked(TBoard::TMove* moves);
	TBoardBatch();
	void MakeMove(const TBoard::TMove& m);
	void MakeMove(const TBoard::TMove& m, const string& s);
	void UndoMove(const TBoard::TMove& m);
	void PrintStory() const;

private:
	vector<TBoard> Boards;
};