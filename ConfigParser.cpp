#include "ConfigParser.h"
#include <tuple>
#include <functional>

namespace std
{
	std::string to_string(vec3 v)
	{
		return std::to_string(v.x()) + " " + std::to_string(v.y()) + " " + std::to_string(v.z());
	}

	std::string to_string(std::string s)
	{
		return s;
	}
}

namespace
{
	constexpr auto noValidation = [](auto v) { return true; };

	std::string getAssetPath(const std::string& originalPath)
	{
		return "_assets/" + originalPath;
	}

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
		auto stringToInt = [](const char *string) -> int { return std::stoi(string); };
		auto validateDimension = [](int value) { return value > 0; };
		return findValue<int>(txt, prefix, stringToInt, validateDimension);
	}

	std::optional<bool> findBool(std::string_view txt, const std::string &prefix)
	{
		auto boolFromYesNo = [](const char *string) { return string[0] == 'y' ? true : false; };

		return findValue<bool>(txt, prefix, boolFromYesNo, noValidation);
	}

	std::optional<vec3> findVec3(std::string_view txt, const std::string& prefix)
	{
		auto stringToVec3 = [](const char *string) -> vec3
		{
			const size_t stringLength = strlen(string);
			vec3 result{};
			const char *current = string;
			char *next;
			for(size_t i = 0; i < result.size(); i++)
			{
				result[i] = std::strtof(current, &next);
				current = next;
			}
			return result;
		};

		return findValue<vec3>(txt, prefix, stringToVec3, noValidation);
	}

	std::optional<std::string> findString(std::string_view txt, const std::string &prefix)
	{
		auto stringParse = [](const char* string) -> std::string
		{
			const std::string result = string;
			const size_t foundIndex = result.find_first_of('\r');
			if(foundIndex != std::string::npos)
			{
				return std::string(result.begin(), result.begin()+foundIndex);
			} else
			{
				return result;
			}
		};

		return findValue<std::string>(txt, prefix, stringParse, noValidation);
	}

	std::optional<float> findFloat(std::string_view txt, const std::string &prefix)
	{
		auto parseFloat = [](const char *string) -> float
		{
			return std::strtof(string, nullptr);
		};

		return findValue<float>(txt, prefix, parseFloat, noValidation);
	}
}

std::optional<PathTracerConfig> parseConfig()
{
	FileReader configReader("_assets/config.txt");
	const std::string configTxt = configReader.readInto<std::string>();

	auto x = findInt(configTxt, "x:");
	auto y = findInt(configTxt, "y:");
	auto spp = findInt(configTxt, "spp:");
	auto tiles = findInt(configTxt, "tiles:");

	auto lookAt = findVec3(configTxt, "lookat:");
	auto lookFrom = findVec3(configTxt, "lookfrom:");
	auto aperture = findFloat(configTxt, "aperture:");
	auto distanceToFocus = findFloat(configTxt, "distancetofocus:");
	auto fov = findFloat(configTxt, "fov:");
	auto modelPath = findString(configTxt, "model:");
	auto albedoPath = findString(configTxt, "texture_albedo:");
	auto emissivePath = findString(configTxt, "texture_emissive:");
	auto normalPath = findString(configTxt, "texture_normal:");
	auto roughnessPath = findString(configTxt, "texture_roughness:");

	//assumed to be false if not found
	auto window = findBool(configTxt, "window:");
	auto writeToFile = findBool(configTxt, "writetofile:");

	if (allHaveValue(x, y, tiles, spp, lookAt, lookFrom, aperture, distanceToFocus, fov, modelPath, albedoPath, emissivePath, normalPath, roughnessPath))
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

			.lookFrom = lookFrom.value(),
			.lookAt = lookAt.value(),
			.distanceToFocus = distanceToFocus.value(),
			.aperture = aperture.value(),
			.camera = Camera(
				result.lookFrom,
				result.lookAt, 
				vec3(.0f, 1.0f, .0f), 
				fov.value(), 
				result.xDim / float(result.yDim), 
				result.aperture, 
				result.distanceToFocus),

			.modelPath = getAssetPath(modelPath.value()),
			.albedoPath = getAssetPath(albedoPath.value()),
			.emissivePath = getAssetPath(emissivePath.value()),
			.normalPath = getAssetPath(normalPath.value()),
			.roughnessPath = getAssetPath(roughnessPath.value()),

			.renderingToScreen = window.value_or(false),
			.writeToFile = writeToFile.value_or(false),
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
