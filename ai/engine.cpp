#include "engine.h"

#include <board/move_serializer.h>

#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

#define LOG_AI(x) cerr << x;
#define PRINT(x) cout << x;

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
					curScore = -Dfs(ti, Moves[ti], { depth - 1, 1, 3 }, -INF, -curScore, dcnt);
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
	if (score > 100)
		score--;
	else if (score < -100)
		score++; 
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
			n = Boards.GenerateMovesUnchecked(FirstMoves) - FirstMoves;
	        ms = TMoveSerializer(FirstMoves, n, Boards[0]);
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

void TEngine::MakeComputerMove(int posCntLim, vector<int> useFactors) {
	for (auto& h : Heuristics)
		h.UseFactors = useFactors;
	TBoard::TMove moves[100];
	if (Boards[0].CurrentPositionWasCount() == 3) {
		for (int bi = 0; bi < THREAD_COUNT; bi++)
			Boards[bi].Status = TBoard::GS_DRAW;
		return;
	}
	int n = Boards.GenerateMovesUnchecked(moves) - moves;
	TMoveSerializer ms(moves, n, Boards[0]);
	vector<bool> valid;
	valid.resize(n);
	vector<int> validIndecies;
	for (int i = 0; i < n; i++) {
		Boards[0].MakeMove(moves[i]);
		valid[i] = !Boards[0].IsOpKingUnderAttack();
		if (valid[i])
			validIndecies.push_back(i);
		Boards[0].UndoMove(moves[i]);
	}
	vector<int> scores(n, 0);
	vector<int> scoresSum(n, 0);
	const int startDepth = 3;
	int depth = startDepth;
	int coeff = 1;
	int cnt;
	int max1 = -INF, max2 = -INF;
	while (!validIndecies.empty()) {
		auto delta = max1 - max2;
		max1 = max2 = -INF;
		LOG_AI("depth = " << depth << endl)
		cnt = 0;
		for (int i : validIndecies) {
			Boards.MakeMove(moves[i]);
			int loc;
			if (depth > startDepth)
				loc = Dfs(depth, cnt, scores[i] - delta, scores[i] + delta);
			else
				loc = Dfs(depth, cnt);
			LOG_AI(ms.GetMoveName(i) << ": " << -loc << endl)
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
		LOG_AI("cnt = " << cnt << endl)
		depth++;
		coeff *= 2;
		if (cnt < posCntLim)
			continue;
		if (cnt < posCntLim * 3 && validIndecies.size() > 3) {
			//cerr << "Secondary ranking will be applied\n";
			sort(validIndecies.begin(), validIndecies.end(), [&](int lhs, int rhs){
				return scoresSum[lhs] < scoresSum[rhs];
			});
			validIndecies.resize(3);
			continue;
		}
		break;
	}
	
	vector<int> iobs; 
	for (int i : validIndecies) {
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
		auto status = Boards[0].IsMyKingUnderAttack() ? TBoard::GS_LOSE : TBoard::GS_DRAW;
		for (int bi = 0; bi < THREAD_COUNT; bi++)
			Boards[bi].Status = status;
		return;
	}
	int iob = iobs[Rand(iobs.size())];
	PRINT("Computer move: " << ms.GetMoveName(iob) << endl)
	Boards.MakeMove(moves[iob], ms.GetMoveName(iob));
	//Boards[0].PrintStory();
}

void TEngine::Print() const {
	Boards[0].Print();
}

void TEngine::MakeSpecialAction() {
	string s;
	while (true) {
		getline(cin, s);
		if (s.empty()) {
			return;
		}
		if (s == "story") {
			Boards[0].PrintStory();
		} else if (s == "print") {
			Boards[0].Print();
		} else if (s == "undo") {
			Boards.Undo();
		} else {
			cout << "Unknown action: " << s;
		}
	}
}
