#include "genetic.h"

#include <board/board.h>
#include <board/move_serializer.h>
#include <genetic/genetic.h>

#include <util.h>

#include <iostream>

using namespace std;
using namespace NBoard;

vector<vector<int>> PlayTournament(const vector<vector<int>>& members) {
    return {};
/*	vector<double> scores(members.size(), 0.0);
	cerr << "Members:\n";
	for (int i = 0; i < members.size(); i++) {
		cerr << i << ":";
		for (int c : members[i])
			cerr << " " << c;
		cerr << endl;		
	}
    cerr << " ";
    for (int i = 0; i < members.size(); i++)
        cerr << i;
    cerr << endl;
	for (int wi = 0; wi < members.size(); wi++) {
        string info;
        info += (char)(wi + '0');
        for (int bi = 0; bi < members.size(); bi++) {
    		if (wi == bi) {
                info += '\\';
                continue;
    		}
    		TBoardBatch boards;
    		TEngine engine(boards);
    		int turn = 1;
    		bool isWhite = true;
    		while (boards[0].UpdateStatus() == NBoard::GS_PLAY) {
    			//engine.MakeComputerMove(5e4, members[isWhite ? wi : bi]);
                engine.MakeComputerMoveBetter(2e7, members[isWhite ? wi : bi]);
                isWhite = !isWhite;
    			if (isWhite) turn++;
    			cerr << "\r" << info << turn;
    			if (turn == 150) {
    				break;
    			}   				
    		}
            cerr << "\r" << info << "   ";
    		if (turn == 150 || boards[0].Status != NBoard::GS_LOSE) {
    			info += '.';
                scores[wi] += 0.5;
    			scores[bi] += 0.5;
    		} else {
    			if (isWhite) turn--;
    			double winRes = 0.75 + 0.25 / turn;
    			scores[wi] += isWhite ? 1.0 - winRes : winRes;
    			scores[bi] += isWhite ? winRes : 1.0 - winRes;
                info += isWhite ? '-' : '+';
    		}
	    }
        cerr << "\r" << info << endl;
    }
	cerr << "Tournament completed\n";
	cerr << "scores: \n";
	int iob = 0, iosb = 1;
	if (scores[iosb] > scores[iob])
		swap(iob, iosb);
	for (int i = 0; i < members.size(); i++) {
		if (i > 1) {
			if (scores[i] > scores[iob]) {
				iosb = iob;
				iob = i;
			} else if (scores[i] > scores[iosb]) {
				iosb = i;
			}
		}
		for (int c : members[i])
			cerr << c << " ";
		cerr << "=> " << scores[i] << endl;
	}
	return { members[iob], members[iosb] };
    */
}

vector<int> CreateGibrid(const vector<int>& p1, const vector<int>& p2) {
    vector<int> res;
	for (int i = 0; i < (int)p1.size(); i++) {
		int l = min(p1[i], p2[i]);
        int r = max(p1[i], p2[i]);
        if (l == r) {
            l = max(0, l-1);
            r = min(16, r + 1);
        }
        res.push_back(l + Rand(r - l + 1));
    }
	return res;
}

vector<int> CreateChild(const vector<int>& p) {
	vector<int> res;
	for (int x : p) {
		int r = Rand(4);
		if (r == 0) {
			res.push_back(max(0, x-1));
		} else if (r == 3) {
			res.push_back(min(16, x+1));
		} else {
			res.push_back(x);
		}
	}
	return res;
}

static bool HasDup(const vector<vector<int>>& a) {
    for (int i = a.size() - 2; i >= 0; i--)
        if (a.back() == a[i])
            return true;
    return false;
}

vector<vector<int>> CreateMembers(const vector<vector<int>>& parents) {
	vector<vector<int>> members;
	members.push_back(parents[0]);
	members.push_back(parents[1]);
    cerr << "Generatig members" << endl;
    members.push_back(members.back());
    while (HasDup(members)) {
        cerr << 2 << " ";
        members.pop_back();
        members.push_back(CreateGibrid(parents[0], parents[1]));
    }
    
    members.push_back(members.back());
    while (HasDup(members)) {
        cerr << 3 << " ";
        members.pop_back();
        members.push_back(CreateGibrid(parents[0], parents[1]));
	}

    members.push_back(members.back());
    while (HasDup(members)) {
        cerr << 4 << " ";
        members.pop_back();
        members.push_back(CreateChild(parents[0]));
	}

    members.push_back(members.back());
    while (HasDup(members)) {
        cerr << 5 << " ";
        members.pop_back();
        members.push_back(CreateChild(parents[0]));
    }
	
    members.push_back(members.back());
    while (HasDup(members)) {
        cerr << 6 << " ";
        members.pop_back();
        members.push_back(CreateChild(parents[1]));
    }

    members.push_back(members.back());
    while (HasDup(members)) {
        cerr << 7 << " ";
        members.pop_back();
    	members.push_back(CreateChild(parents[1]));
	}
    cerr << endl;
    return members;
}

void RunGenetic() {
	vector<vector<int>> bestMembers = {
		// 7 2 0 0 1 1 1 1 2
        { 9, 4, 1, 0, 1, 0, 1, 1, 2, 1, 1 },
        { 9, 4, 1, 0, 1, 0, 1, 1, 2, 1, 0 }
        //{ 4, 1, 0, 1, 0, 3, 0, 0, 1 },
		//{ 4, 0, 4, 0, 1, 0, 4, 0, 0 }
	};
	for (int itr = 0; itr < 10; itr++) {
		cerr << "Tournament #" << itr + 1 << endl;
		bestMembers = PlayTournament(CreateMembers(bestMembers));
	}
	cin.get();
	cin.get();
}
