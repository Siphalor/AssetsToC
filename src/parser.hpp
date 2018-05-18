#ifndef PARSER_HPP
#define PARSER_HPP
#include <ctype.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <map>
#include <boost/filesystem.hpp>
#include "generator.hpp"
#include "log.hpp"

#define ATOC_PARSER_GENERATE_FILES atoc::generator::generateFiles(input, output, settings, forceGen)

namespace atoc::parser {

enum class ParseMode {
	await, config, genInput, genMiddle, genOutput, inlineConfig, include, inputDirectory, outputDirectory, comment, cmd
};

enum class Phase {
	normal, file, var, compression, key, value, specialize, next
};

struct FilePosition {
	unsigned line = 1;
	unsigned column = 0;
};

static bool isGenForced(boost::filesystem::path cache, boost::filesystem::path atoc);
bool parseFile(boost::filesystem::path inputFile, boost::filesystem::path outputDir);

bool isCharOneOf(char& needle, const char* haystack);

namespace error {
void unexpectedSymbol(FilePosition pos, char symbol, std::string expectation = "", std::string symbolName = "symbol");
void statementEnd(FilePosition pos, std::string expectation = "");
}
}

#endif
