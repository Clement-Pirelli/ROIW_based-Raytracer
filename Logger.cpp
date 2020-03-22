#include "Logger.h"
#include <iostream>
#include <cstdarg>

void Logger::LogMessage(const char *message)
{
	std::cout << "[message] " << message << '\n';
}

void Logger::LogMessageFormatted(const char * const format, ...)
{
	std::cout << "[message] ";

	//formatted argument list
	va_list list;
	va_start(list, format);
	vprintf(format, list);
	va_end(list);

	std::cout << '\n';
}

void Logger::LogMessage(const char *message, int line, const char *file)
{
	std::cout << "[message] " << message << " (line: " << line << ", file: " << file <<")\n";
}

void Logger::LogError(const char *error)
{
	std::cerr << "[error!!!] " << error << '\n';
}

void Logger::LogErrorFormatted(const char *format, ...)
{
	std::cout << "[error!!!] ";

	//formatted argument list
	va_list list;
	va_start(list, format);
	vprintf(format, list);
	va_end(list);

	std::cout << '\n';
}

void Logger::LogError(const char *error, int line, const char *file)
{
	std::cerr << "[error!!!] " << error << " (line: " << line << ", file: " << file << ")\n";
}
