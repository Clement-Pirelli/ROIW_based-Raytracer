#include "ConfigParser.h"
#include <tuple>
#include <functional>

namespace
{
	template<typename T, typename Format_t, typename Validate_t>
	std::optional<T> findValue(std::string_view haystack, const std::string &prefix, Format_t format, Validate_t validate)
	{
		size_t position = haystack.find(prefix);
		if (position == std::string::npos)
		{
			Logger::LogErrorFormatted("Could not find value for prefix '%s'!", prefix.c_str());
			return std::nullopt;
		}

		const T value = format(haystack.data() + position + prefix.size());

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

	std::optional<int> findInt(std::string_view txt, const std::string& prefix)
	{
		auto stringToInt = [](const char *string) { return std::stoi(string); };
		auto validateDimension = [](int value) { return value > 0; };
		return findValue<int>(txt, prefix, stringToInt, validateDimension);
	}

	std::optional<bool> findBool(std::string_view txt, const std::string &prefix)
	{
		auto boolFromYesNo = [](const char *string) { return string[0] == 'y' ? true : false; };
		auto boolAllValid = [](bool v) { return true; };

		return findValue<bool>(txt, prefix, boolFromYesNo, boolAllValid);
	}
}

std::optional<PathTracerConfig> parseConfig()
{
	FileReader configReader("_assets/config.txt");
	std::string configTxt;
	configTxt.resize(configReader.calculateLength());
	configReader.read(configTxt.data(), configTxt.size());

	const std::optional<int> x = findInt(configTxt, "x:");
	const std::optional<int> y = findInt(configTxt, "y:");
	const std::optional<int> spp = findInt(configTxt, "spp:");
	const std::optional<int> tiles = findInt(configTxt, "tiles:");
	
	//assumed to be false if not found
	const std::optional<bool> window = findBool(configTxt, "window:");
	const std::optional<bool> writeToFile = findBool(configTxt, "writetofile:");

	if (allHaveValue(x, y, tiles, spp))
	{
		const PathTracerConfig result
		{
			.xDim = (size_t)x.value(),
			.yDim = (size_t)y.value(),
			.samples = (size_t)spp.value(),
			.dimTileAmount = (size_t)tiles.value(),
			.tileWidth = result.xDim / result.dimTileAmount,
			.tileHeight = result.yDim / result.dimTileAmount,
			.totalTileAmount = result.dimTileAmount * result.dimTileAmount,
			.renderingToScreen = window.value_or(false),
			.writeToFile = window.value_or(false)
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
