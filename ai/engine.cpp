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

int TEngine::Dfs(int depth, int& cnt, int a, int b) {
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
					curScore = -Dfs(ti, Moves[ti], { depth - 1, 2, 7 }, -INF, -curScore, dcnt);
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

int TEngine::Dfs(int ti, TBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt) {
	cnt++;
	auto& board = Boards[ti];
	if (board.CurrentPositionWasCount() >= 3) {
		return 0;
	}
	if (lim.Depth == 0) {
		return Heuristics[ti].GetScore();
	}
	int score = -INF - 1;
	auto movesEnd = board.GenerateMovesUnchecked(moves);
	vector<int> indeciesOfValid;
	int i = 0;
	for (auto m = moves; m != movesEnd; m++) {
		board.MakeMove(*m);
		if (!board.IsOpKingUnderAttack())
			indeciesOfValid.push_back(i);
		board.UndoMove(*m);
		i++;
	}
	for (int i : indeciesOfValid) {
		int s = -INF - 1;
		board.MakeMove(moves[i]);
		if (moves[i].IsCapturing() && lim.Capture > 0)
			s = -Dfs(ti, movesEnd, { lim.Depth, lim.Capture - 1, lim.Force }, -b, -a, cnt);
		else if ((int)indeciesOfValid.size() <= lim.Force)
			s = -Dfs(ti, movesEnd, { lim.Depth, lim.Capture, lim.Force - (int)indeciesOfValid.size() }, -b, -a, cnt);
		else
			s = -Dfs(ti, movesEnd, { lim.Depth - 1, lim.Capture, lim.Force }, -b, -a, cnt);			
		board.UndoMove(moves[i]);
		if (s > -INF) {
			if (s >= b) {
				return s;
			}
			score = max(s, score);
			a = max(a, score);
		}
	}
	if (score < -INF) {
		return board.IsMyKingUnderAttack() ? -INF : 0;
	}
	return score;
}

void TEngine::MakeUserMove() {
	int n = Boards.GenerateMovesUnchecked(FirstMoves) - FirstMoves;
	TMoveSerializer ms(FirstMoves, n, Boards[0]);
	int i;
	do {
		string s;
		cout << "Your move: ";
		cin >> s;
		if (s == "sv") {
			ms.ShowValid();
			continue;
		} else if (s == "story") {
			Boards.PrintStory();
			cout << endl;
			continue;
		} else if (s == "undo") {
			Boards.Undo();
			Boards.Undo();
			continue;
		} else if (s == "print") {
			Boards[0].Print();
			continue;
		} else if (s == "debug_moves") {
			for (int i = 0; i < n; i++) {
				if (ms.GetMoveName(i) == "ba1") {
					FirstMoves[i].Print();
				}
			}
		}
		i = ms.GetMoveIndex(s);
	} while (i == -1);

	Boards.MakeMove(FirstMoves[i], ms.GetMoveName(i));
}

void TEngine::MakeComputerMove(int posCntLim) {
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
	vector<int> scoresSum(n, 0);
	const int startDepth = 3;
	int depth = startDepth;
	int coeff = 1;
	int cnt;
	int max1 = -INF, max2 = -INF;
	do {
		auto delta = max1 - max2;
		max1 = max2 = -INF;
		cerr << "depth = " << depth << endl;
		cnt = 0;
		for (int i = 0; i < n; i++) {
			if (valid[i]) {
				Boards.MakeMove(moves[i]);
				int loc;
				if (depth > startDepth)
					loc = Dfs(depth, cnt, scores[i] - delta, scores[i] + delta);
				else
					loc = Dfs(depth, cnt);
				cerr << ms.GetMoveName(i) << ": " << -loc << endl;
				scores[i] = loc;
				if (-loc > max1) {
					max2 = max1;
					max1 = -loc; 
				} else if (-loc > max2) {
					max2 = -loc;
				}
				scoresSum[i] += scores[i] * coeff;
				Boards.UndoMove(moves[i]);
			}
		} 
		cerr << "cnt = " << cnt << endl;
		depth++;
		coeff *= 2;
	} while (cnt < posCntLim);
	vector<int> iobs; 
	for (int i = 0; i < n; i++) {
		if (!valid[i])
			continue;
		if (iobs.empty()) {
			iobs.push_back(i);
		} else if (scoresSum[i] < scoresSum[iobs[0]]) {
			iobs.clear();
			iobs.push_back(i);
		} else if (scoresSum[i] == scoresSum[iobs[0]]) {
			iobs.push_back(i);
		}
	}
	if (iobs.empty()) {
		cout << "Seems that game is over" << endl;
		return;
	}
	int iob = iobs[Rand(iobs.size())];
	cout << "Computer move: " << ms.GetMoveName(iob) << endl;
	Boards.MakeMove(moves[iob], ms.GetMoveName(iob));
}

void TEngine::Print() const {
	Boards[0].Print();
}