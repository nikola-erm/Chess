#include "util.h"
#include "config.h"

mt19937 rnd(RANDOM_SEED);

int Rand(int n) {
	return rnd() % n;
}

string CallStackNames[100];
size_t CallStackSize = 0;
