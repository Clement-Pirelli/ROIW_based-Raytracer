#include <thread>
#include "PathTracer.h"
#include "Logger.h"
#include "ConfigParser.h"
#include <optional>

int main()
{
	Logger::LogMessageFormatted("Welcome to the raytracer! You have %u available threads.\n", std::thread::hardware_concurrency());
	std::optional<PathTracerConfig> config = parseConfig();
	if(config)
	{
		PathTracer trace(config.value());
	}
}