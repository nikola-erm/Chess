#include "genetic.h"

#include <ai/engine.h>
#include <board/board.h>
#include <board/move_serializer.h>
#include <genetic/genetic.h>

#include <util.h>

#include <iostream>

using namespace std;

vector<vector<int>> PlayTournament(const vector<vector<int>>& members) {
	vector<double> scores(members.size(), 0.0);
	cerr << "Members:\n";
	for (int i = 0; i < members.size(); i++) {
		cerr << i << ":";
		for (int c : members[i])
			cerr << " " << c;
		cerr << endl;		
	}
	for (int wi = 0; wi < members.size(); wi++)
	for (int bi = 0; bi < members.size(); bi++) {
		if (wi == bi) continue;
		cerr << "Game " << wi << " vs " << bi << endl;
		TBoardBatch boards;
		TEngine engine(boards);
		int turn = 1;
		bool isWhite = true;
		while (boards[0].Status == TBoard::GS_PLAY) {
			engine.MakeComputerMove(5e4, members[isWhite ? wi : bi]);
			isWhite = !isWhite;
			if (isWhite) turn++;
			cerr << ".";
			if (turn == 150) {
				break;
			}
				
		}
		cerr << endl;
		boards[0].PrintStory();
		cerr << endl;
		if (boards[0].Status != TBoard::GS_LOSE) {
			scores[wi] += 0.5;
			scores[bi] += 0.5;
		} else {
			if (isWhite) turn--;
			double winRes = 0.75 + 0.25 / turn;
			scores[wi] += isWhite ? 1.0 - winRes : winRes;
			scores[bi] += isWhite ? winRes : 1.0 - winRes;
		}
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
}

vector<int> CreateGibrid(const vector<int>& p1, const vector<int>& p2) {
    vector<int> res;
	for (int i = 0; i < p1.size(); i++)
		res.push_back(Rand(2) ? p1[i] : p2[i]);
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

vector<vector<int>> CreateMembers(const vector<vector<int>>& parents) {
	vector<vector<int>> members;
	members.push_back(parents[0]);
	members.push_back(parents[1]);
	members.push_back(CreateGibrid(parents[0], parents[1]));
	members.push_back(CreateGibrid(parents[0], parents[1]));
	members.push_back(CreateChild(parents[0]));
	members.push_back(CreateChild(parents[0]));
	members.push_back(CreateChild(parents[1]));
	members.push_back(CreateChild(parents[1]));
	return members;
}

void RunGenetic() {
	vector<vector<int>> bestMembers = {
		{ 9, 0, 1, 0, 1, 1, 1, 0 },
		{ 8, 1, 0, 0, 0, 0, 0, 1 }
		//THeuristics::DefaultUseFactors,
		//{ 8, 0, 0, 0, 0, 0, 0, 0 }
	};
	for (int itr = 0; itr < 10; itr++) {
		cerr << "Tournament #" << itr + 1 << endl;
		bestMembers = PlayTournament(CreateMembers(bestMembers));
	}
	cin.get();
	cin.get();
}
