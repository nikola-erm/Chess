#pragma once

#include <board/board.h>
#include <unordered_map>
#include <vector>
#include <string>

namespace NBoard {

class TMoveSerializer {
public:
	TMoveSerializer(const TMove* moves, int n, TBoard& board);
	int GetMoveIndex(const std::string& s) const;
	const std::string& GetMoveName(int i) const;
	void ShowValid() const;

private:
	std::vector<bool> IsValid;
	std::vector<std::vector<std::string>> Names;
	std::vector<int> CanonIndecies;
	std::unordered_map<std::string, std::vector<int>> NameToValid, NameToInvalid;

	static std::vector<std::string> GenerateNames(const TBoard& board, const TMove& m);
};

} // NBoard