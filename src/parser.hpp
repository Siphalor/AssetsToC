#ifndef PARSER_HPP
#define PARSER_HPP
#include <ctype.h>
#include <cstring>
#include <string>
#include <fstream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "generator.hpp"
#include "log.hpp"

#define ATOC_PARSER_GENERATE_FILES atoc::generator::generateFiles(input, output, settings, forceGen)

namespace atoc::parser {

enum class ParseMode {
	await, config, genInput, genMiddle, genOutput, inlineConfig, include, inputDirectory, outputDirectory
};

enum class Phase {
	normal, file, var, key, value, specialize, next
};

bool isGenForced(boost::filesystem::path cache, boost::filesystem::path atoc);
bool parseFile(boost::filesystem::path inputFile, boost::filesystem::path outputDir);

bool isCharOneOf(char& needle, const char* haystack);

namespace error {
void unexpectedSymbol(unsigned line, char symbol, std::string expectation = "", std::string symbolName = "symbol");
void statementEnd(unsigned line, std::string expectation = "");
}
}

#endif
