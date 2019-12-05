#include "engine.h"
#include "estimate_tree_size.h"

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

TEngine::TDfsResult TEngine::Dfs(int depth, int& cnt, int a, int b) {
	cnt++;
	int movesCount = Boards.GenerateMovesUnchecked(FirstMoves) - FirstMoves;
	vector<thread> threads;
	TDfsResult res = { -INF - 1, -INF - 1 };
	mutex scoreMutex;
	for (int ti = 0; ti < THREAD_COUNT; ti++)
		threads.push_back(thread([&](int ti){
			int dcnt = 0;
			for (int i = ti; i < movesCount; i += THREAD_COUNT) {
				Boards[ti].MakeMove(FirstMoves[i]);
				if (!Boards[ti].IsOpKingUnderAttack()) {
					TDfsResult curRes;
					{
						lock_guard<mutex> guard(scoreMutex);
						curRes = res;
					}
    				curRes = -Dfs(ti, Moves[ti], { depth - 1, 2, 7 }, -b, -a, dcnt, false);
					{
						lock_guard<mutex> guard(scoreMutex);
						if (curRes.Score > res.Score) {
                            res.StumbleScore = max(res.Score, res.StumbleScore);
                            res.Score = curRes.Score;    
                        }
                        if (curRes.StumbleScore < res.StumbleScore) {
                            res.StumbleScore = curRes.StumbleScore;
                        }
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
	if (res.Score < -INF)
		return Boards[0].IsMyKingUnderAttack() ? TDfsResult{-INF, -INF} : TDfsResult{ 0, 0 };
    if (res.StumbleScore < -INF) {
        res.StumbleScore = res.Score;
    }
	return res;
}

TEngine::TDfsResult TEngine::Dfs(int ti, TBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt, bool opTurn) {
	cnt++;
	auto& board = Boards[ti];
	if (board.CurrentPositionWasCount() >= 3) {
		return {0, 0};
	}
	if (lim.Depth == 0) {
		int s = Heuristics[ti].GetScore();
        return { s, s };
	}
	TDfsResult res = { -INF - 1, -INF - 1 };
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
		TDfsResult curRes;
		board.MakeMove(moves[i]);
		if (moves[i].IsCapturing() && lim.Capture > 0)
			curRes = -Dfs(ti, movesEnd, { lim.Depth, lim.Capture - 1, lim.Force }, -b, -a, cnt, !opTurn);
		else if ((int)indeciesOfValid.size() <= lim.Force)
			curRes = -Dfs(ti, movesEnd, { lim.Depth, lim.Capture, lim.Force - (int)indeciesOfValid.size() }, -b, -a, cnt, !opTurn);
		else
			curRes = -Dfs(ti, movesEnd, { lim.Depth - 1, lim.Capture, lim.Force }, -b, -a, cnt, !opTurn);			
		board.UndoMove(moves[i]);
        if (curRes.Score > res.Score) {
            if (opTurn)
                res.StumbleScore = max(res.Score, res.StumbleScore);
            res.Score = curRes.Score;    
        }
        if (curRes.StumbleScore < res.StumbleScore) {
            res.StumbleScore = curRes.StumbleScore;
        }
		if (res.Score > b) {
            if (res.StumbleScore < -INF) {
                res.StumbleScore = res.Score;
            }
			return res;
		}
	}
	if (indeciesOfValid.empty()) {
		return board.IsMyKingUnderAttack() ? TDfsResult{ -INF, -INF } : TDfsResult{ 0, 0 };
	}
    if (res.StumbleScore < -INF) {
        res.StumbleScore = res.Score;
    }
	return res;
}

int TEngine::DfsFixed(int ti, TBoard::TMove* moves, int depth, long long& cnt, int a) {
    if (a > INF - depth - 2)
        a++;
    if (a < -INF + depth + 2)
        a--;
    cnt++;
	auto& board = Boards[ti];
	if (board.CurrentPositionWasCount() >= 3) {
		return 0;
	}
	if (depth == 0) {
		int s = Heuristics[ti].GetScore();
        return s;
	}
	int res = -INF - 1;                               
	auto movesEnd = board.GenerateMovesUnchecked(moves);
	int movesCnt = movesEnd - moves;
    if (depth > 3) {
        vector<int> iov;
        vector<int> scr(movesCnt);
        for (int i = 0; i < movesCnt; i++) {
            board.MakeMove(moves[i]);
            if (!board.IsOpKingUnderAttack()) {
                iov.push_back(i);
                scr[i] = Heuristics[ti].GetScore();
            }
            board.UndoMove(moves[i]);
        }
        sort(iov.begin(), iov.end(), [&](int lhs, int rhs){ return scr[lhs] < scr[rhs]; });
        for (int i : iov) {
            int curRes = -INF - 1;
            board.MakeMove(moves[i]);
            curRes = -DfsFixed(ti, movesEnd, depth - 1, cnt, -a + 1);
            board.UndoMove(moves[i]);
            res = max(res, curRes);
            if (res >= a)
                return res;
        }
    } else {
	    for (int i = 0; i < movesCnt; i++) {
		    int curRes = -INF - 1;
		    board.MakeMove(moves[i]);
            if (!board.IsOpKingUnderAttack()) {
			    curRes = -DfsFixed(ti, movesEnd, depth - 1, cnt, -a + 1);			
	        }
    	    board.UndoMove(moves[i]);
            res = max(res, curRes);
            if (res >= a)
                return res;
	    }
    }
	if (res < -INF) {
		return board.IsMyKingUnderAttack() ? -INF : 0;
	}
    if (res < 100 - INF)
        res++;
    else if (res > INF - 100)
        res--;
    else if (depth == 1)
        res = (Heuristics[ti].GetScore() + 2 * res) / 3;
    return res;
}

void TEngine::MakeComputerMoveBetter(long long posCntLim, vector<int> useFactors) {
    for (auto& h : Heuristics)
        h.UseFactors = useFactors;
    TBoard::TMove moves[100];
	int n = Boards.GenerateMovesUnchecked(moves) - moves;
	TMoveSerializer ms(moves, n, Boards[0]);
    vector<int> iobs;
	for (int i = 0; i < n; i++) {
		Boards[0].MakeMove(moves[i]);
		if (!Boards[0].IsOpKingUnderAttack())
            iobs.push_back(i);
        Boards[0].UndoMove(moves[i]);
	} 
    int a = 0;
    int step = 0;
    bool speedUp = true;
    vector<vector<double>> cntByDepth;
    cntByDepth.resize(n);
    for (int i : iobs) {
        Boards[0].MakeMove(moves[i]);
        cntByDepth[i] = EstimateTreeSize(Boards[0], 25);
        Boards[0].UndoMove(moves[i]);
    }
    int itrCnt = 0;
    while (iobs.size() > 1) {
        itrCnt++; 
        bool breakImm = ((step == -1 || step == 1) && !speedUp);            
        int depth = 11;
        while (depth > 3) {
            double tec = 0;
            for (int i : iobs)
                tec += cntByDepth[i][depth-1];
            if (tec * (breakImm ? 0.5 : 1.0) > posCntLim)
                depth--; 
            else
                break;
        }
        if (itrCnt > 10)
            depth--;
        LOG_AI("a = " << a << "; step = " << step << "; depth = " << depth << "\n")
		vector<int> newIobs;
        long long cnt[] = {0, 0, 0, 0};
        if (breakImm)
            random_shuffle(iobs.begin(), iobs.end());    
        int ii = 0;
        mutex iiMutex, newIobsMutex, printMutex;
        vector<thread> threads;


        for (int ti = 0; ti < min(THREAD_COUNT, (int)iobs.size()); ti++) {
            threads.push_back(thread([&](int ti) {
                while (true) {
                    int i;
                    {
                        lock_guard<mutex> guard(iiMutex);
                        if (ii == iobs.size())
                            break;
                        i = iobs[ii];
                        ii++;
                    }
                    Boards[ti].MakeMove(moves[i]);
    			    int score = -DfsFixed(ti, Moves[ti], depth, cnt[ti], -a + 1);            
                    {
                        lock_guard<mutex> guard(printMutex);
                        LOG_AI(ms.GetMoveName(i) << ": " << score << " => " << (score >= a ? "good" : "weak") << endl)
                    }
                    //cerr << ".";
                    {
                        lock_guard<mutex> guard(newIobsMutex);
                        if (score >= a) {
                            newIobs.push_back(i);
                        }
			            Boards[ti].UndoMove(moves[i]);
                        if (!newIobs.empty() && breakImm) {
                            //iobs = newIobs;
                            break;
                        }
                    }
                }
            }, ti));
		}
        for (auto& t : threads)
            t.join();
        
        int tcnt = 0;
        for (int c : cnt)
            tcnt += c;
        LOG_AI("total cnt: " << tcnt << endl)
        LOG_AI(endl)
        if (!newIobs.empty()) {
            iobs = newIobs;                                                                                        
            if (step == 0) {
                step = 1;
                a += step;
                continue;
            }
            if (step < 0) {
                step = -step;
                speedUp = false;
            }
        } else {
            if (step == 0) {
                step = -1;
                a += step;
                continue;
            }
            if (step > 0) {
                step = -step;
                speedUp = false;
            }
        }
        if (speedUp)
            step *= 2;
        else
            step /= 2;
        if (step == 0)
            break;
        a += step;
    }

	if (iobs.empty()) {
		PRINT("Seems that game is over" << endl)
		return;
	}
	int iob = iobs[Rand(iobs.size())];
	PRINT("Computer move: " << ms.GetMoveName(iob) << endl)
	Boards.MakeMove(moves[iob], ms.GetMoveName(iob));
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

/*
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
*/


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

TEngine::TDfsResult operator -(const TEngine::TDfsResult& score) {
    return { -score.Score, -score.StumbleScore };
}

TEngine::TDfsResult TEngine::TDfsResult::GetNormalized() const {
    return { Score, min(Score + 100, max(Score - 100, StumbleScore)) };
}
