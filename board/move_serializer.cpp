#include "move_serializer.h"

#include <util.h>

using namespace std;
using namespace NBoard;

const string& TMoveSerializer::GetMoveName(int i) const {
	const auto& ret = Names.at(i).at(CanonIndecies.at(i));
	return ret;
}

TMoveSerializer::TMoveSerializer(const TMove* moves, int n, TBoard& board) {
	Names.resize(n);
	CanonIndecies.resize(n);
	IsValid.resize(n);
	
	for (int mi = 0; mi < n; mi++) {
		board.MakeMove(moves[mi]);
		IsValid[mi] = !board.IsOpKingUnderAttack();
		board.UndoMove(moves[mi]);
		Names[mi] = GenerateNames(board, moves[mi]);
		if (IsValid[mi]) {
			for (const auto& name : Names[mi])
				NameToValid[name].push_back(mi);
		} else {
			for (const auto& name : Names[mi])
				NameToInvalid[name].push_back(mi);
		}
	}

	for (int mi = 0; mi < n; mi++) {
		int& i = CanonIndecies[mi] = 0;
		while (true) {
			const auto& name = Names[mi][i];
			if (NameToValid[name].size() + NameToInvalid[name].size() == 1)
				break;
			i++;
		}
	}
}

static void AddTransformFigure(vector<string>& res, EMaskType maskType) {
	char fig = '?';
	switch (maskType) {
	case MT_WQUEEN:
	case MT_BQUEEN:
		fig = 'Q';
		break;
	case MT_WROOK:
	case MT_BROOK:
		fig = 'R';
		break;
	case MT_WBISHOP:
	case MT_BBISHOP:
		fig = 'B';
		break;
	case MT_WKNIGHT:
	case MT_BKNIGHT:
		fig = 'N';
		break;
    default:
        assert(false);        					
	}
	for (auto& s : res) {
		s += fig;
	}
}

vector<string> TMoveSerializer::GenerateNames(const TBoard& board, const TMove& m) {
	const auto& fstr = TBoard::FieldStr;
	TMask maskFrom = (m.Masks[0] & board.Masks[m.MaskTypes[0]]);
	TMask maskTo = maskFrom ^ m.Masks[0];
	if (maskTo == 0)
		maskTo = m.Masks[1];
	int from = GetBitPos(maskFrom);
	int to = GetBitPos(maskTo);
	string fig;
	switch (m.MaskTypes[0]) {
	case MT_WKING:
	case MT_BKING:
		if (fstr[from][0] == 'e' && fstr[to][0] == 'g')
			return { "0-0" };
		else if (fstr[from][0] == 'e' && fstr[to][0] == 'c')
			return { "0-0-0" };
		else
			return { "K" + fstr[to] };
		break;
	case MT_WROOK:
	case MT_BROOK:
		fig = "R";
		break;
	case MT_WQUEEN:
	case MT_BQUEEN:
		fig = "Q";
		break;
	case MT_WBISHOP:
	case MT_BBISHOP:
		fig = "B";
		break;
	case MT_WKNIGHT:
	case MT_BKNIGHT:
		fig = "N";
		break;
	case MT_WPAWN:
	case MT_BPAWN:
		if (fstr[from][0] != fstr[to][0]) {
		 	vector<string> ret = { fstr[from].substr(0, 1) + fstr[to], fstr[from].substr(0, 1) + fstr[to].substr(0, 1), fstr[to] };
		 	if (!(maskTo & m.Masks[0]))
		 		AddTransformFigure(ret, m.MaskTypes[2]);
		 	return ret;
		} 
		break;
	default:
		assert(false);				
	}
	vector<string> res;
	for (const string& cp : (m.IsCapturing() ? vector<string>({ "x", "" }) : vector<string>({ "" }))) {
		res.push_back(fig + cp + fstr[to]);
		res.push_back(fig + fstr[from][0] + cp + fstr[to]);
		res.push_back(fig + fstr[from] + cp + fstr[to]);
	}
	if (m.MaskTypes[0] == MT_WPAWN || m.MaskTypes[0] == MT_BPAWN) {
		if (!(maskTo & m.Masks[0]))
		 	AddTransformFigure(res, m.MaskTypes[1]);
	}
	return res;
}

int TMoveSerializer::GetMoveIndex(const string& s) const {
	if (NameToValid.count(s) > 0 && NameToValid.at(s).size() == 1)
		return NameToValid.at(s)[0];
	if (NameToValid.count(s) == 0 || NameToValid.at(s).size() == 0) {
		if (NameToInvalid.count(s) > 0 && NameToInvalid.at(s).size() > 0) {
			cout << "'" << s << "' matches only invalid moves:";
			for (int i : NameToValid.at(s))
				cout << " " << GetMoveName(i);
			cout << endl;	 
		} else {
			cout << "'" << s << "' doesn't match any move" << endl; 
		}
	} else {
		cout << "'" << s << "' is ambigous between:";
		for (int i : NameToValid.at(s))
			cout << " " << GetMoveName(i); 
		cout << endl;
	}
	return -1;
}

void TMoveSerializer::ShowValid() const {
	for (size_t i = 0; i < IsValid.size(); i++)
		if (IsValid[i])
			cout << GetMoveName(i) << " ";
	cout << endl;
}