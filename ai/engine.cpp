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

TEngine::TEngine(NBoard::TBoardBatch& boards)
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

TEngine::TDfsResult TEngine::Dfs(int ti, NBoard::TMove* moves, TDfsLimits lim, int a, int b, int& cnt, bool opTurn) {
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

int TEngine::DfsFixed(int ti, NBoard::TMove* moves, int depth, long long& cnt, int a) {
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
    NBoard::TMove moves[100];
	int n = Boards.GenerateMovesUnchecked(moves) - moves;
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
	NBoard::TMoveSerializer ms(FirstMoves, n, Boards[0]);
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
	        ms = NBoard::TMoveSerializer(FirstMoves, n, Boards[0]);
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

int TEngine::DfsFixedScore(int depth, int ti) {
	int l = 0, r = 0;
    long long cnt;
    if (DfsFixed(ti, Moves[ti], depth, cnt, 0) >= 0) {
        r = 1;
        while (r < INF && DfsFixed(ti, Moves[ti], depth, cnt, r) >= r)
            r = r * 2 + 1;
    } else {
        l = -1;
        while (l >= -INF && DfsFixed(ti, Moves[ti], depth, cnt, l) < l)
            l = l * 2 - 1;
    }
	while (l < r) {
		int m = l + (r - l) / 2;
		if (DfsFixed(ti, Moves[ti], depth, cnt, m+1) >= m+1)
			l = m+1;
		else
			r = m;
	}
	return l;
}

int TEngine::DfsFixedScore(int depth, int ti, int l, int r) {
    l--;
    r++;
    long long cnt;
	while (l < r) {
		int m = l + (r - l) / 2;
		if (DfsFixed(ti, Moves[ti], depth, cnt, m+1) >= m+1)
			l = m+1;
		else
			r = m;
	}
	return l;
}

void TEngine::BuildGameTreeStructure(TGameTreeNode& v, int depth) {
    v.MinScore = -INF;
    v.MaxScore = INF;
    if (depth == 0)
        return;
    static NBoard::TMove moves[100];
    int n = Boards[0].GenerateMoves(moves) - moves;
    if (n == 0 || Boards[0].CurrentPositionWasCount() == 3)
        return;
    v.Children.resize(n);
    NBoard::TMoveSerializer ms(moves, n, Boards[0]);
    for (int i = 0; i < n; i++) {
        v.Children[i].Move = moves[i];
        v.Children[i].MoveName = ms.GetMoveName(i);
    }
    for (int i = 0; i < n; i++) {
        Boards[0].MakeMove(v.Children[i].Move);
        BuildGameTreeStructure(v.Children[i], depth-1);
        Boards[0].UndoMove(v.Children[i].Move);
    }
}

void TEngine::BuildLeafsForUpdateList(TGameTreeNode& v, vector<vector<TGameTreeNode*>>& result, vector<TGameTreeNode*>& leafStack, int a) {
    leafStack.push_back(&v);
    if (v.MinScore < a  && a <= v.MaxScore) {
        if (v.Children.empty()) {
            result.push_back(leafStack);
        } else {
            for (int i = 0; i < (int) v.Children.size(); i++) {
                Boards[0].MakeMove(v.Children[i].Move);
                BuildLeafsForUpdateList(v.Children[i], result, leafStack, -a + 1);
                Boards[0].UndoMove(v.Children[i].Move);
            }
        }
    }
    leafStack.pop_back();
}

bool TEngine::NeedEstimate(const vector<TGameTreeNode*>& seq, int a) {
    for (const TGameTreeNode* x : seq) {
        if (x->MinScore >= a || x->MaxScore < a)
            return false;
        a = -a + 1;
    }
    return true;
}

template<typename T>
static void RandomShuffle(vector<T>& seq) {
    int n = seq.size();
    for (int i = 0; i < n; i++) {
        int j = Rand(n);
        swap(seq[i], seq[j]);
    }
}

void TEngine::EstimateLeafs(vector<vector<TGameTreeNode*>>& leafs, int depth, int a) {
    //RandomShuffle(leafs);
    int ii = 0;
    mutex iMutex;
    mutex tMutex;
    vector<thread> threads;
    for (int ti = 0; ti < THREAD_COUNT; ti++)
        threads.push_back(thread([&](int ti, int a) {
            long long cnt;
            while (true) {
                int i;  
                {
                    lock_guard<mutex> guard(iMutex);
                    i = ii;
                    ii++;
                }
                if (i >= (int) leafs.size())
                    break;
                if (ti == 0)
                    LOG_AI("\rEstimating leafs " << (i * 100 / leafs.size()) << " %   ")
                auto& seq = leafs[i];

                {
                    lock_guard<mutex> guard(tMutex);
                    if (!NeedEstimate(seq, a))
                        continue;
                }

                for (int j = 1; j < (int) seq.size(); j++) {
                    Boards[ti].MakeMove(seq[j]->Move);
                    a = -a + 1;
                }
                int score = DfsFixed(ti, Moves[ti], depth, cnt, a);
                if (score >= a)
                    seq.back()->MinScore = a;
                else
                    seq.back()->MaxScore = a-1;
                {
                    lock_guard<mutex> guard(tMutex);
                    for (int j = seq.size() - 1; j > 0; j--) {
                        Boards[ti].UndoMove(seq[j]->Move);
                        if (seq[j-1]->MinScore < a || seq[j-1]->MaxScore >= a) {
                            bool hasStrong = false;   
                            bool allWeak = true;
                            for (int ci = 0; ci < (int)seq[j-1]->Children.size(); ci++) {
                                if (seq[j-1]->Children[ci].MaxScore < a) {
                                    hasStrong = true;
                                    break;
                                }
                                if (seq[j-1]->Children[ci].MinScore < a) {
                                    allWeak = false;
                                }
                            }
                            a = -a + 1;
                            if (hasStrong)
                                seq[j-1]->MinScore = a;
                            else if (allWeak)
                                seq[j-1]->MaxScore = a-1;
                        } else {
                            a = -a + 1;
                        }
                    }
                }
            }
        }, ti, a));

    for (auto& t : threads)
        t.join();

    LOG_AI("\rEstimating leafs 100 %   " << endl)
} 

TEngine::TGameTreeNode TEngine::BuildGameTree() {
    LOG_AI("Estimating tree size..." << endl)
	auto cntByDepth = EstimateTreeSize(Boards[0], 1000);
	int primaryDepth = 4;
	while (primaryDepth > 3 && cntByDepth[primaryDepth] > 1e5)
		primaryDepth--;
	int secondaryDepth = 8;
	while (secondaryDepth > 2 && cntByDepth[secondaryDepth + primaryDepth] > 3e10)
		secondaryDepth--;
	TGameTreeNode root;
    LOG_AI("Building game tree structure..." << endl)
    BuildGameTreeStructure(root, primaryDepth);
    for (int version = 0; version < 3; version++) {
        if (version > 0) {
            int l = -1, r = 2 * INF;
            double cf = cntByDepth[primaryDepth + secondaryDepth + 1] / cntByDepth[primaryDepth];
            while (l < r) {
                int m = l + (r - l + 1) / 2;
                if (cf * CalcReEstimateLeafs(root, root.MinScore - m, root.MaxScore + m) < 3e10)
                    l = m;
                else
                    r = m-1;
            }
            LOG_AI("radius = " << r << endl)
            if (r < 0)
                break;
            int minScore = root.MinScore - r;
            int maxScore = root.MaxScore + r;
            if (IsTrivial(root, minScore, maxScore)) {
                LOG_AI("one branch => break" << endl)
                break;
            }
            RelaxScores(root, minScore, maxScore);
            secondaryDepth++;
        }
        LOG_AI("Root estimate = [" << root.MinScore << ", " << root.MaxScore << "]" << endl)
        while (root.MinScore < root.MaxScore) {
            int a = root.MinScore + (root.MaxScore - root.MinScore + 1) / 2;
            LOG_AI("a = " << a << "; depth = " << primaryDepth << " + " << secondaryDepth << endl)
            vector<TGameTreeNode*> leafStack;
            vector<vector<TGameTreeNode*>> leafs;
            BuildLeafsForUpdateList(root, leafs, leafStack, a);
            LOG_AI("Leafs count = " << leafs.size() << endl)
            EstimateLeafs(leafs, secondaryDepth, a);
            LOG_AI("Root estimate = [" << root.MinScore << ", " << root.MaxScore << "]" << endl)
        }
    }
    return root;
}

void TEngine::MakeMonteCarloMove(vector<int> useFactors) {
    Heuristics[0].UseFactors = useFactors;
    NBoard::TMove bestMove;
    string bestMoveName;
    if (MonteCarlo.MakeMove(Boards[0], Heuristics[0], bestMove, bestMoveName)) {
        PRINT("Computer move: " << bestMoveName << endl)
        Boards.MakeMove(bestMove);
    } else {
        PRINT("Seems that game is over" << endl)
    }
}

void TEngine::MakeComputerMoveTwoStage(vector<int> useFactors) {
    for (auto& h : Heuristics)
        h.UseFactors = useFactors;
    
    auto root = BuildGameTree();
    
    if (root.Children.empty()) {
        PRINT("Seems that game is over" << endl)
        return;
    }
    int best;
    vector<int> iobs;
    for (int i = 0; i < (int) root.Children.size(); i++) {
        if (iobs.empty() || root.Children[i].MaxScore < best) {
            iobs.clear();
            best = root.Children[i].MaxScore;
        } 
        if (root.Children[i].MaxScore == best) {
            iobs.push_back(i);
        }
    }
    int i = iobs[Rand(iobs.size())];
    LOG_AI("Explanation:")
    Explain(root);
    PRINT("Computer move: " << root.Children[i].MoveName << endl)
    Boards.MakeMove(root.Children[i].Move, root.Children[i].MoveName);
}

void TEngine::Explain(const TGameTreeNode& v, int d) {
    if (v.Children.empty()) {
        LOG_AI("\n") 
        return;
    }
    string tab(d, '\t');
    int best = v.Children[0].MaxScore;
    for (const auto& c : v.Children)
        best = min(best, c.MaxScore);
    LOG_AI("[\n")
    for (const auto& c : v.Children) {
        if (c.MaxScore != best)
            continue;
        LOG_AI(tab << "\t" << c.MoveName)
        Explain(c, d+1);    
    }

    LOG_AI(tab << "]\n")
}

void TEngine::TGameTreeNode::Print() const {
    cerr << MoveName << " [" << MinScore << ", " << MaxScore << "]";
    if (Children.empty())
        return;
    cerr << " { ";
    for (const auto& v : Children) {
        v.Print();
        cerr << " ";
    }
    cerr << "}";
}

bool TEngine::IsTrivial(const TGameTreeNode& v, int minScore, int maxScore) {
    bool has = false;
    swap(minScore, maxScore);
    minScore *= -1;
    maxScore *= -1;
    for (const auto& c : v.Children) {
        if (maxScore < c.MinScore || c.MaxScore < minScore)
            continue;
        if (has)
            return false;
        has = true;
    }
    return true;
}

void TEngine::RelaxScores(TGameTreeNode& v, int minScore, int maxScore) {
    if (maxScore < v.MinScore || v.MaxScore < minScore)
        return;
    v.MinScore = min(v.MinScore, minScore);
    v.MaxScore = max(v.MaxScore, maxScore);
    for (auto& c : v.Children)
        RelaxScores(c, -maxScore, -minScore);
}

int TEngine::CalcReEstimateLeafs(const TGameTreeNode& v, int minScore, int maxScore) {
    if (v.MaxScore < minScore || maxScore < v.MinScore)
        return 0;
    if (v.Children.empty())
        return 1;
    int sum = 0;
    for (const auto& c : v.Children)
        sum += CalcReEstimateLeafs(c, -maxScore, -minScore);
    return sum;
}

int TEngine::TGameTreeNode::CalcNumberOfInteresting(int a, int b) const {
    if (MaxScore < a || MinScore > b)
        return 0;
    if (Children.empty())
        return 1;
    int sum = 0;
    for (int i = 0; i < (int) Children.size(); i++)
        sum += Children[i].CalcNumberOfInteresting(-b, -a);
    return sum;
}

bool TEngine::TGameTreeNode::IsTrivial(int a, int b) const {
    int ioi = -1;
    for (int i = 0; i < (int) Children.size(); i++) {
        if (Children[i].MinScore > -b && Children[i].MaxScore < -a)
            continue;
        if (ioi != -1)
            return false;
        ioi = i;
    }
    return true;            
}

void TEngine::BuildLeafList(TGameTreeNode& v, vector<vector<TGameTreeNode*>>& result, vector<TGameTreeNode*>& leafStack) {
    leafStack.push_back(&v);
    if (v.Children.empty()) {
        result.push_back(leafStack);
    } else {
        for (int i = 0; i < (int) v.Children.size(); i++)
            BuildLeafList(v.Children[i], result, leafStack);
    }
    leafStack.pop_back();
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
