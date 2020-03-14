#ifndef LOGGER_H_DEFINED
#define LOGGER_H_DEFINED

class Logger
{
public:

	static void LogMessage(const char *message);
	static void LogMessageFormatted(const char *format, ...);
	static void LogMessage(const char *message, int line, const char *file);
	static void LogError(const char *error);
	static void LogErrorFormatted(const char *format, ...);
	static void LogError(const char *error, int line, const char *file);

private:
	Logger() = default;
};


#endif
