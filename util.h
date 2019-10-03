#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include <iostream>

using namespace std;
using TMask = unsigned long long;

int Rand(int n);

inline unsigned long long Now() {
	using namespace std::chrono;
	return duration_cast< milliseconds >(
   		system_clock::now().time_since_epoch()
	).count();
}

const int INF = 1e6;

#define BCNT(x) __builtin_popcountll(x)

inline int Num(int x, int y) {
	return x + (y << 3);
}

inline TMask GetMask(int x, int y) {
	return 1ull << Num(x, y);
}

inline TMask GetMask(int pos) {
	return 1ull << pos;
}

inline bool Valid(int x, int y) {
	return 0 <= x && x < 8 && 0 <= y && y < 8;
}

inline TMask LowestBit(const TMask& m) {
	return m & (~(m-1));
}

inline int GetBitPos(TMask m) {
	int res = 0;
	if (m & 0xffffffff00000000) {
		res |= 32;
		m >>= 32;
	}
	if (m & 0xffff0000) {
		res  |= 16;
		m >>= 16;
	}
	if (m & 0xff00) {
		res |= 8;
		m >>= 8;
	}
	if (m & 0xf0) {
		res |= 4;
		m >>= 4;
	}
	switch (m) {
		case 1:
			return res;
		case 2:
			return 1 | res;
		case 4:
			return 2 | res;
		case 8:
			return 3 | res;
	}
	cerr << "invalid bit pos" << endl;
	return -1;
}

#define PRINT_LINE(x, n)                            \
		cerr << ((x >> (n * 8 + 0)) & 1)            \
			 << ((x >> (n * 8 + 1)) & 1)            \
			 << ((x >> (n * 8 + 2)) & 1)            \
			 << ((x >> (n * 8 + 3)) & 1)            \
			 << ((x >> (n * 8 + 4)) & 1)            \
			 << ((x >> (n * 8 + 5)) & 1)            \
			 << ((x >> (n * 8 + 6)) & 1)            \
			 << ((x >> (n * 8 + 7)) & 1) << endl;
			
#define PRINT_MASK_IMPL(x)               \
		cerr << #x << endl;         \
		PRINT_LINE(x, 7)               \
		PRINT_LINE(x, 6)               \
		PRINT_LINE(x, 5)               \
		PRINT_LINE(x, 4)               \
		PRINT_LINE(x, 3)               \
		PRINT_LINE(x, 2)               \
		PRINT_LINE(x, 1)               \
		PRINT_LINE(x, 0)	            \
		cerr << endl;

#define PRINT_MASK(x) PRINT_MASK_IMPL(Masks[x])
