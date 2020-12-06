#include <thread>
#include "PathTracer.h"
#include "Logger.h"
#include <iostream>
#include "Files.h"
#include <optional>

template<typename T, typename Format_t, typename Validate_t>
std::optional<T> findValue(const std::string& haystack, const std::string &prefix, Format_t format, Validate_t validate)
{
	size_t position = haystack.find(prefix);
	if(position == std::string::npos)
	{
		Logger::LogErrorFormatted("Could not find value for prefix '%s'!", prefix.c_str());
		return std::nullopt;
	}

	const T value = format(haystack.c_str() + position + prefix.size());

	if(!validate(value))
	{
		Logger::LogErrorFormatted("Value for prefix '%s' is not valid!", prefix.c_str());
		return std::nullopt;
	}

	Logger::LogMessageFormatted("%s %u", prefix.c_str(), value);

	return value;
}

std::optional<PathTracerConfig> readConfig()
{
	PathTracerConfig config;
	config.threadAmount = std::thread::hardware_concurrency() - 1;

	FileReader configReader("_assets/config.txt");
	std::string configString;
	configString.resize(configReader.calculateLength());
	configReader.read(configString.data(), configString.size());

	auto stringToInt = [](const char *string) { return std::stoi(string); };
	auto validateDimension = [](int value) { return value > 0; };

	const std::optional<int> x = findValue<int>(configString, "x:", stringToInt, validateDimension);
	config.xDim = x.value_or(0);

	const std::optional<int> y = findValue<int>(configString, "y:", stringToInt, validateDimension);
	config.yDim = y.value_or(0);

	const std::optional<int> spp = findValue<int>(configString, "spp:", stringToInt, validateDimension);
	config.samples = spp.value_or(0);

	const std::optional<int> tiles = findValue<int>(configString, "tiles:", stringToInt, validateDimension);
	config.dimTileAmount = tiles.value_or(0);

	if (!(config.xDim >= config.dimTileAmount && config.yDim >= config.dimTileAmount)) 
	{
		Logger::LogError("Invalid tile value! Please enter a value smaller than or equal to the amount of pixels!");
		return std::nullopt;
	}

	std::optional<char> window = findValue<char>(configString, "window:", [](const char *string) { return string[0]; }, [](char value) { return true; });
	config.renderingToScreen = window.value_or('n') == 'y' ? true : false;

	if (!x.has_value() || !y.has_value() || !tiles.has_value() || !spp.has_value()) 
	{
		return std::nullopt;
	} else 
	{
		config.totalTileAmount = config.dimTileAmount * config.dimTileAmount;
		config.tileWidth = config.xDim / config.dimTileAmount;
		config.tileHeight = config.yDim / config.dimTileAmount;
		return config;
	}
}

int main()
{
	Logger::LogMessageFormatted("Welcome to the raytracer! You have %u available threads.\n", std::thread::hardware_concurrency());
	std::optional<PathTracerConfig> config = readConfig();
	if(config)
	{
		PathTracer pathTracer(std::move(config.value()));
		pathTracer.run();
	}
}