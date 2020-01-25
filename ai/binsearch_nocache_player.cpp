#include "binsearch_nocache_player.h"

#include "estimate_tree_size.h"

#include <board/move_serializer.h>

#include <mutex>
#include <thread>

using namespace NAI;

TBinSearchNoCachePlayer::TBinSearchNoCachePlayer(double posCntLim)
    : PosCntLim(posCntLim)
{
    for (int i = 0; i < THREAD_COUNT; i++)
		Heuristics.emplace_back(Boards[i]);
}

void TBinSearchNoCachePlayer::Undo() {
    Boards.Undo();
}

int TBinSearchNoCachePlayer::ChooseMove() {
    NBoard::TMove moves[100];
	int n = Boards[0].GenerateMoves(moves);
	NBoard::TMoveSerializer ms(moves, n, Boards[0]);
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
        if (a > INF)
            break;
        itrCnt++; 
        bool breakImm = ((step == -1 || step == 1) && !speedUp);            
        int depth = 15;
        while (depth > 3) {
            double tec = 0;
            for (int i : iobs)
                tec += cntByDepth[i][depth-1];
            if (tec * (breakImm ? 0.5 : 1.0) > PosCntLim)
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
                        if (ii == (int)iobs.size())
                            break;
                        i = iobs[ii];
                        ii++;
                    }
                    Boards[ti].MakeMove(moves[i]);
    			    int score = -DfsFixed(ti, Moves[ti], depth, cnt[ti], -a + 1);            
                    {
                        lock_guard<mutex> guard(printMutex);
                        LOG_AI(ms.GetMoveName(i) << ": " << score << " => " << (score >= a ? "good" : "weak") << " (thread " << ti << ")" << endl)
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

    assert(!iobs.empty());

	int iob = iobs[Rand(iobs.size())];
	PRINT("Computer move: " << ms.GetMoveName(iob) << endl)
    return iob;
}

void TBinSearchNoCachePlayer::MakeMove(int i) {
    NBoard::TMove moves[100];
    Boards[0].GenerateMoves(moves);
    Boards.MakeMove(moves[i]);
}

int TBinSearchNoCachePlayer::DfsFixed(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a) {
    if (a > INF - depth - 2)
        a++;
    if (a < -INF + depth + 2)
        a--;
    cnt++;
	auto& board = Boards[ti ];
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