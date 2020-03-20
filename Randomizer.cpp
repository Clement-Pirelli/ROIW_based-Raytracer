#include "Randomizer.h"
#include "randUtils.h"


std::vector<vec3> Randomizer::randoms = {};

void Randomizer::createRandom(int number)
{
	randoms.resize(number);

	// set the initial first coordinate
	float x = drand48();
	float min = x;
	unsigned int idx = 0;
	// set the first coordinates
	for (unsigned int i = 0; i < number; ++i) {
		randoms[i][1] = x;
		// keep the minimum
		if (x < min)
			min = x, idx = i;
		// increment the coordinate
		x += 0.618033988749894f;
		if (x >= 1) --x;
	}

	// find the first Fibonacci >= N
	unsigned int f = 1, fp = 1, parity = 0;
	while (f + fp < number) {
		unsigned int tmp = f; f += fp, fp = tmp;
		++parity;
	}

	// set the increment and decrement
	unsigned int inc = fp, dec = f;
	if (parity & 1)
		inc = f, dec = fp;

	// permute the first coordinates
	randoms[0][0] = randoms[idx][1];
	for (unsigned int i = 1; i < number; ++i) {
		if (idx < dec) {
			idx += inc;
			if (idx >= number) idx -= dec;
		}
		else
			idx -= dec;
		randoms[i][0] = randoms[idx][1];
	}
	// set the initial second coordinate
	float y = drand48();
	// set the second coordinates
	for (unsigned int i = 0; i < number; ++i) {
		randoms[i][1] = y;
		// increment the coordinate
		y += 0.618033988749894f;
		if (y >= 1) --y;
	}
}

vec3 Randomizer::getRandom(int sample)
{
	return randoms[sample];
}
