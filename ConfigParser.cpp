#include "ConfigParser.h"
#include <tuple>
#include <functional>

namespace
{
	template<typename T, typename Format_t, typename Validate_t>
	std::optional<T> findValue(const std::string &haystack, const std::string &prefix, Format_t format, Validate_t validate)
	{
		size_t position = haystack.find(prefix);
		if (position == std::string::npos)
		{
			Logger::LogErrorFormatted("Could not find value for prefix '%s'!", prefix.c_str());
			return std::nullopt;
		}

		const T value = format(haystack.c_str() + position + prefix.size());

		if (!validate(value))
		{
			Logger::LogErrorFormatted("Value for prefix '%s' is not valid!", prefix.c_str());
			return std::nullopt;
		}

		Logger::LogMessageFormatted("%s %s", prefix.c_str(), std::to_string(value).c_str());

		return value;
	}

	//just because I can :P
	template<typename... Args>
	bool allHaveValue(Args&&... args)
	{
		auto hasValue = []<typename T>(T && opt) { return opt.has_value(); };
		auto values = {hasValue(args)...};
		return std::all_of(values.begin(), values.end(), [](bool b) {return b; });
	}
}

std::optional<PathTracerConfig> parseConfig()
{
	FileReader configReader("_assets/config.txt");
	std::string configString;
	configString.resize(configReader.calculateLength());
	configReader.read(configString.data(), configString.size());

	auto stringToInt = [](const char *string) { return std::stoi(string); };
	auto validateDimension = [](int value) { return value > 0; };

	const std::optional<int> x = findValue<int>(configString, "x:", stringToInt, validateDimension);
	
	const std::optional<int> y = findValue<int>(configString, "y:", stringToInt, validateDimension);
	
	const std::optional<int> spp = findValue<int>(configString, "spp:", stringToInt, validateDimension);
	
	const std::optional<int> tiles = findValue<int>(configString, "tiles:", stringToInt, validateDimension);
	
	std::optional<bool> window = findValue<bool>(configString, "window:", [](const char *string) { return string[0] == 'y' ? true : false; }, [](bool value) { return true; });

	if (allHaveValue(x, y, tiles, spp))
	{
		const PathTracerConfig result
		{
			.xDim = (size_t)x.value_or(0),
			.yDim = (size_t)y.value_or(0),
			.samples = (size_t)spp.value_or(0),
			.dimTileAmount = (size_t)tiles.value_or(0),
			.tileWidth = result.xDim / result.dimTileAmount,
			.tileHeight = result.yDim / result.dimTileAmount,
			.totalTileAmount = result.dimTileAmount * result.dimTileAmount,
			.threadAmount = std::thread::hardware_concurrency() - 1,
			.renderingToScreen = window.value_or(false),
		};

		if (!(result.xDim >= result.dimTileAmount && result.yDim >= result.dimTileAmount))
		{
			Logger::LogError("Invalid tile value! Please enter a value smaller than or equal to the amount of pixels!");
			return std::nullopt;
		}

		return result;
	} else 
	{
		return std::nullopt;
	}
}
