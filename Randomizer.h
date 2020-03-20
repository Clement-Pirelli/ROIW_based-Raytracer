#ifndef RANDOMIZER_H_INCLUDED
#define RANDOMIZER_H_INCLUDED

#include "vec3.h"
#include <vector>

class Randomizer
{
public:

	static void createRandom(int number);
	static vec3 getRandom(int sample);

private:

	static std::vector<vec3> randoms;
	Randomizer() = delete;
};


#endif