#ifndef FILES_H_DEFINED
#define FILES_H_DEFINED
#include <fstream>
#include <string>
#include <iostream>
#include <assert.h>
#include <vector>

class FileReader {
public:
	FileReader(std::string givenPath) : path(givenPath), stream(path, std::ios::binary)
	{
		assert(!stream.fail() && stream.is_open());
	}

	~FileReader()
	{
		stream.close();
	}

	[[nodiscard]]
	size_t calculateLength()
	{
		stream.seekg(0, stream.end);
		size_t length = stream.tellg();
		stream.seekg(0, stream.beg);
		return length;
	}

	void read(char *dataToFill, size_t size)
	{
		assert(dataToFill != nullptr);
		stream.read(dataToFill, size);
	}

private:
	std::string path;
	std::ifstream stream;
};

class FileWriter {
public:
	FileWriter(std::string givenPath) : path(givenPath), stream(path, std::ios::binary) {}

	~FileWriter()
	{
		stream.close();
	}

	template<typename T>
	[[nodiscard]]
	bool write(const T &data)
	{
		return writeInternal(reinterpret_cast<const char *>(&data), sizeof(T));
	}

	template<typename T>
	[[nodiscard]]
	bool writeVector(const std::vector<T> &data)
	{
		return writeInternal(reinterpret_cast<const char *>(data.data()), sizeof(T) * data.size());
	}

private:

	bool writeInternal(const char *data, size_t size)
	{
		stream.write(data, size);
#ifndef NDEBUG
		if (stream.fail()) return false;
#endif
		return true;
	}

	std::string path;
	std::ofstream stream;
};


#endif