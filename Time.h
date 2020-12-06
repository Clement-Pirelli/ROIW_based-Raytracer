#ifndef TIME_H_DEFINED
#define TIME_H_DEFINED
#include <chrono>

class Time
{
public:
	struct milliseconds
	{
		long long amount = 0;
	};

	Time(milliseconds originalAmount = { 0 })
	{
		ticks = originalAmount;
	}

	//https://stackoverflow.com/questions/31255486/c-how-do-i-convert-a-stdchronotime-point-to-long-and-back
	static milliseconds milliseconds_since_epoch()
	{
		auto now = std::chrono::system_clock::now();
		auto millisecondsNow = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		auto epoch = millisecondsNow.time_since_epoch();
		return { epoch.count() };
	}

	static float seconds_since_epoch()
	{
		return ((float)milliseconds_since_epoch().amount) / 1000.0f;
	}

	static Time now()
	{
		return Time({ milliseconds_since_epoch().amount }) - startTime;
	}

	float asSeconds() const
	{
		return ((float)asMilliseconds().amount) / 1000.0f;
	}

	milliseconds asMilliseconds() const
	{
		return ticks;
	}

	[[nodiscard]]
	Time operator-(const Time &other) const
	{
		return Time({ ticks.amount - other.ticks.amount });
	}

	void operator-=(const Time &other)
	{
		ticks.amount -= other.ticks.amount;
	}

	void operator+=(const Time &other)
	{
		ticks.amount += other.ticks.amount;
	}

	void operator-=(float other)
	{
		ticks.amount -= (long long)other;
	}

	void operator+=(float other)
	{
		ticks.amount += (long long)other;
	}

	[[nodiscard]]
	Time operator+(const Time &other) const
	{
		return Time({ ticks.amount + other.ticks.amount });
	}

	static const Time startTime;

private:

	milliseconds ticks = {};
};

#endif