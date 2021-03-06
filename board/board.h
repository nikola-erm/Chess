#pragma once

#include"common.h"

#include <config.h>

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

using TMask = unsigned long long;

class THeuristics;

namespace NBoard {

enum EGameStatus {
	GS_PLAY,
	GS_LOSE,
	GS_DRAW
};

struct TMove {
	EMaskType MaskTypes[5];
	TMask Masks[5];
	TMask HashMask;
	int Count;

	void Reset();
	void Add(EMaskType maskType, int pos);
	void Add(EMaskType maskType, int pos1, int pos2);
	void AddHash(TMask mask);
	bool IsCapturing() const;
    void Print() const;
};

class TBoard {
public:
	friend class ::THeuristics;
	friend class TMoveSerializer;
    friend class TMove;
    friend class TBoardMovesIterator;

public:
	TBoard();
	TBoard(const string& fen);

	void MakeMove(const TMove& m, const string& name);
	void MakeMove(const TMove& m);
	void UndoMove(const TMove& m);
	void Undo();

	TMove* GenerateMovesUnchecked(TMove* moves);
    int GenerateMoves(TMove* moves);
	
	bool IsOpKingUnderAttack() const;
	bool IsMyKingUnderAttack() const;
	bool IsUnderOpAttack(int pos) const;
	int CurrentPositionWasCount() const;

	void Print() const;
	void PrintStory() const;
	
    //Slow!
    EGameStatus UpdateStatus();
	EGameStatus Status;
    int GetTurn() const;

private:
	int Turn;
	TMask Masks[14];
	TMove Story[10000];
	string StoryNames[10000];
    EMaskType OpMt[64];
	unordered_map<TMask, int> WasCount;

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
	void InitFromFEN(const string& fen);
    static void StaticInit();

    static bool StaticInitDone;
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
	static TMask RequiredEnPassantMask[64][2];
	static TMove EnPassantMove[64][2];
	static TMask FigurePrints[14][64];
	static TMask KnightByDistArea[64][3];
	static TMask DiagonalMask[64];
	static int KingCentrality[64];
};

class TBoardBatch {
public:
	TBoard& operator[](int i);
	TMove* GenerateMovesUnchecked(TMove* moves);
	TBoardBatch();
	TBoardBatch(const string& fen);
	void MakeMove(const TMove& m);
	void MakeMove(const TMove& m, const string& s);
	void UndoMove(const TMove& m);
	void Undo();
	void PrintStory() const;

private:
	vector<TBoard> Boards;
};

} // NBoard