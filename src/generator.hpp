#ifndef GENERATOR_HPP
#define GENERATOR_HPP
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#ifdef COMPRESS_ZIP
#include "zlib.h"
#endif // COMPRESS_ZIP
#include "util.hpp"
#include "log.hpp"

namespace atoc::generator {

enum class Compression;
struct OutputInformation;
class Settings;

bool generateFiles(std::map<std::string,OutputInformation> input, std::vector<std::string> output, Settings& settings, bool force = false);
static bool processFile(boost::filesystem::path file, OutputInformation outputInformation, std::ofstream& output, Settings& settings);

std::string generateVarName(std::string input, Settings& settings);
std::string generateOutputFilename(std::string input, Settings& settings);

enum class Compression {
	inherit, none,
#ifdef COMPRESS_ZIP
	zip,
#endif // COMPRESS_ZIP
};
Compression getCompression(std::string id);

struct OutputInformation {
	std::string variableName = "";
	Compression compression = Compression::inherit;
};

class Settings {
	public:
		struct Setting {
			typedef unsigned char Flags;
			typedef uint16_t number_type;
			const static Flags nonzero = 0b1;
			const static Flags notnull = 0b1;
			enum Type { string, number, boolean, fault };
			std::string description;
			Type type;
			void* value;
			Flags flags;
			const std::string toString() const;
		};
		~Settings();
		const static std::map<std::string, Setting> settings;
		static void printHelp();
		bool set(std::string key, std::string value);
		template<typename T> T get(std::string key);
		Setting::Type getType(std::string key);
		void deleteEntry(std::string key);
		void setIO(std::string inputDirectory_, std::string outputDirectory_);
		std::string getInputDirectory() {return inputDirectory;};
		std::string getOutputDirectory() {return outputDirectory;};
	protected:
		std::map<std::string, void*> values;
		std::string inputDirectory;
		std::string outputDirectory;
};

}

#endif
