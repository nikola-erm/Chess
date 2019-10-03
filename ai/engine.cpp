#include "engine.h"

#include <board/move_serializer.h>

#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

using namespace std;

TEngine::TEngine(TBoardBatch& boards)
	: Boards(boards) 
{
	for (int i = 0; i < THREAD_COUNT; i++)
		Heuristics.emplace_back(Boards[i]);
}

int TEngine::Dfs(int depth, int& cnt) {
	cnt++;
	int movesCount = Boards.GenerateMovesUnchecked(FirstMoves) - FirstMoves;
	vector<thread> threads;
	int score = -INF - 1;
	mutex scoreMutex;
	for (int ti = 0; ti < THREAD_COUNT; ti++)
		threads.push_back(thread([&](int ti){
			int dcnt = 0;
			for (int i = ti; i < movesCount; i += THREAD_COUNT) {
				Boards[ti].MakeMove(FirstMoves[i]);
				if (!Boards[ti].IsOpKingUnderAttack()) {
					int curScore;
					{
						lock_guard<mutex> guard(scoreMutex);
						curScore = score;
					}
					curScore = -Dfs(ti, Moves[ti], depth - 1, -INF, -curScore, dcnt);
					{
						lock_guard<mutex> guard(scoreMutex);
						score = max(score, curScore);
					}
				}
				Boards[ti].UndoMove(FirstMoves[i]);	
			}
			{
				lock_guard<mutex> guard(scoreMutex);
				cnt += dcnt;
			}
		}, ti));
	for (auto& t : threads)
		t.join();
	if (score < -INF)
		return Boards[0].IsMyKingUnderAttack() ? -INF : 0;
	return score;
}

int TEngine::Dfs(int ti, TBoard::TMove* moves, int depth, int a, int b, int& cnt) {
	cnt++;
	auto& board = Boards[ti];
	if (depth == 0) {
		return Heuristics[ti].GetScore();
	}
	int score = -INF - 1;
	auto movesEnd = board.GenerateMovesUnchecked(moves);
	for (auto m = moves; m != movesEnd; m++) {
		int s = -INF - 1;
		board.MakeMove(*m);
		if (!board.IsOpKingUnderAttack()) {
			s = -Dfs(ti, movesEnd, depth - 1, -b, -a, cnt);		
		}
		board.UndoMove(*m);
		if (s > -INF) {
			if (s >= b)
				return s;
			score = max(s, score);
			a = max(a, score);
		}
	}
	if (score < -INF)
		return board.IsMyKingUnderAttack() ? -INF : 0;
	return score;
}

void TEngine::MakeUserMove() {
	int n = Boards.GenerateMovesUnchecked(FirstMoves) - FirstMoves;
	TMoveSerializer ms(FirstMoves, n, Boards[0]);
	int i;
	do {
		string s;
		cin >> s;
		if (s == "sv") {
			ms.ShowValid();
			continue;
		}
		i = ms.GetMoveIndex(s);
	} while (i == -1);
	Boards.MakeMove(FirstMoves[i]);
}

void TEngine::MakeComputerMove() {
	TBoard::TMove moves[100];
	int n = Boards.GenerateMovesUnchecked(moves) - moves;
	TMoveSerializer ms(moves, n, Boards[0]);
	vector<bool> valid;
	valid.resize(n);
	for (int i = 0; i < n; i++) {
		Boards[0].MakeMove(moves[i]);
		valid[i] = !Boards[0].IsOpKingUnderAttack();
		Boards[0].UndoMove(moves[i]);
	}
	vector<int> scores(n, 0);
	int depth = 4;
	int coeff = 1;
	int cnt;
	do {
		cerr << "depth = " << depth << endl;
		cnt = 0;
		for (int i = 0; i < n; i++) {
			if (valid[i]) {
				Boards.MakeMove(moves[i]);
				int loc = Dfs(depth, cnt);
				cerr << ms.GetMoveName(i) << ": " << -loc << endl;
				scores[i] += loc * coeff;
				Boards.UndoMove(moves[i]);
			}
		} 
		cerr << "cnt = " << cnt << endl;
		depth++;
		coeff *= 2;
	} while (cnt < (int)1e7);
	int iob = -1;
	for (int i = 0; i < n; i++) {
		if (!valid[i])
			continue;
		if (iob == -1 || scores[i] < scores[iob])
			iob = i;
	}
	if (iob == -1) {
		cout << "Seems that game is over" << endl;
		return;
	}
	cout << ms.GetMoveName(iob) << endl;
	Boards.MakeMove(moves[iob]);
}
