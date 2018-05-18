#include "parser.hpp"

namespace atoc::parser {

bool isGenForced(boost::filesystem::path cache, boost::filesystem::path atoc) {
	if(boost::filesystem::exists(boost::filesystem::status(cache))) {
		if(boost::filesystem::last_write_time(atoc) > boost::filesystem::last_write_time(cache)) return true;
		std::fstream file;
		file.open(cache.string(), std::ios_base::in);
		unsigned short version;
		file >> version;
		if(version > ATOC_VERSION_MAJOR) return true;
		file.get(); file >> version;
		if(version > ATOC_VERSION_MINOR) return true;
		file.get(); file >> version;
		if(version > ATOC_VERSION_SUBMINOR) return true;
		return false;
	} else return true;
};

bool parseFile(boost::filesystem::path inputFile, boost::filesystem::path outputDir) {
	atoc::log::init(atoc::log::Severity::debug, "atoc.log");
	ATOC_LOG(parser, info) << "Test";
	if(!boost::filesystem::exists(boost::filesystem::status(inputFile))) {
		ATOC_LOG(parser, fatal) << "The .atoc file '" << inputFile.wstring() << "' doesn't exist." << std::endl;
		return false;
	}
	ATOC_LOG(parser, info) << ATOC_APP_NAME << " version " << ATOC_VERSION_MAJOR << "." << ATOC_VERSION_MINOR << "." << ATOC_VERSION_SUBMINOR;
	std::fstream file;
	bool forceGen = isGenForced(boost::filesystem::path(inputFile.parent_path().string()+"/.atoccache"), inputFile);
	bool endOfFile = false;
	ParseMode parseMode = ParseMode::await;
	Phase phase;
	atoc::generator::Settings settings;
	settings.setIO(inputFile.parent_path().string(), outputDir.string());
	char symbol = '\0';
	bool bundled = false;
	bool await = true;
	std::map<std::string,atoc::generator::OutputInformation> input;
	std::vector<std::string> output;
	auto currentInputIterator = input.end();
	bool outputGiven = false;
	std::string currentString = "";
	std::string whiteBuffer = "";
	std::string configKey = "";
	FilePosition filePosition;
	file.open(inputFile.string(), std::ios_base::in | std::ios_base::binary);
	while(!endOfFile) {
		if(!isCharOneOf(symbol, ATOC_WHITESPACE))
			whiteBuffer = "";
		if(isCharOneOf(symbol, ATOC_NEWLINE)) {
			filePosition.line++;
			filePosition.column = 0;
			if(parseMode == ParseMode::comment) parseMode = ParseMode::await;
		}
		symbol = file.get();
		filePosition.column++;
		if(symbol == std::char_traits<char>::eof()) {
			endOfFile = true;
			symbol = '\n';
		}
		if(parseMode == ParseMode::comment) continue;
		if(await) {
			if(parseMode == ParseMode::await) {
				if(isCharOneOf(symbol, ATOC_WHITESPACE))
					continue;
				else if(symbol == '$') {
					parseMode = ParseMode::config;
					phase = Phase::specialize;
					continue;
				} else if(symbol == '#') {
					parseMode = ParseMode::comment;
				} else if(symbol == ';')
					continue;
				else if(symbol == '>') {
					parseMode = ParseMode::outputDirectory;
					continue;
				} else if(symbol == '<') {
					parseMode = ParseMode::inputDirectory;
					continue;
				} else {
					ATOC_LOG(parser, debug) << "Awaiting statement...";
					parseMode = ParseMode::genInput;
					phase = Phase::file;
					if(symbol == '{') {
						bundled = true;
						continue;
					} else {
						bundled = false;
						await = false;
					}
				}
			} else if(parseMode == ParseMode::genMiddle) {
				if(symbol == '>') {
					parseMode = ParseMode::genOutput;
					bundled = false;
					continue;
				} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
					continue;
				} else if(symbol == ';') {
					error::statementEnd(filePosition, "automatic output filename generation is not available for bundles");
					return false;
				} else {
					error::unexpectedSymbol(filePosition, symbol, "expected '>' or statement end");
					return false;
				}
			} else if(parseMode == ParseMode::genOutput) {
				if(isCharOneOf(symbol, ATOC_WHITESPACE))
					continue;
				else if(symbol == '{' && !bundled) {
					bundled = true;
					continue;
				} else if(bundled && symbol == '}') {
					await = false;
				} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
					error::unexpectedSymbol(filePosition, symbol, "expected output filename", "operator");
					return false;
				} else {
					await = false;
				}
			} else if(parseMode == ParseMode::config || parseMode == ParseMode::inlineConfig) {
				if(isCharOneOf(symbol, ATOC_NEWLINE)) {
					if(phase == Phase::key) {
						parseMode = ParseMode::await;
						continue;
					} else {
						await = false;
					}
				} else if(isCharOneOf(symbol, ATOC_WHITESPACE))
					continue;
				else {
					await = false;
				}
			} else if(parseMode == ParseMode::inputDirectory || parseMode == ParseMode::outputDirectory) {
				if(isCharOneOf(symbol, ATOC_NEWLINE)) {
					if(parseMode == ParseMode::inputDirectory)
						settings.set("input-dir", "");
					else settings.set("output-dir", "");
					parseMode = ParseMode::await;
					continue;
				} else if(isCharOneOf(symbol, ATOC_WHITESPACE))
					continue;
				else
					await = false;
			} else {
				if(isCharOneOf(symbol, ATOC_WHITESPACE))
					continue;
				await = false;
			}
		}
		switch(parseMode) {
			case ParseMode::inlineConfig:
			case ParseMode::config:
				if(phase == Phase::specialize) {
					if(symbol == '$') {
						parseMode = ParseMode::inlineConfig;
						phase = Phase::key;
						await = true;
						continue;
					} else if(symbol == '<') {
						parseMode = ParseMode::include;
					} else if(symbol == '>') {
						parseMode = ParseMode::cmd;
						await = true;
						continue;
					} else if(isCharOneOf(symbol, ATOC_NEWLINE)) {
						error::statementEnd(filePosition, "expected config statement");
						return false;
					} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
						error::unexpectedSymbol(filePosition, symbol, "expected config statement", "operator");
						return false;
					} else {
						phase = Phase::key;
						await = true;
					}
				} 
				if(phase == Phase::key) {
					if(symbol == ':') {
						if(currentString == "") {
							error::unexpectedSymbol(filePosition, symbol, "expected config key");
							return false;
						}
						configKey = currentString;
						currentString = "";
						phase = Phase::value;
						await = true;
					} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
						error::unexpectedSymbol(filePosition, symbol, "expected config key", "operator");
						return false;
					} else if(isCharOneOf(symbol, ATOC_INLINE_STATEMENT_END)) {
						error::statementEnd(filePosition, "expected config key");
						return false;
					} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
						whiteBuffer += symbol;
					} else {
						currentString += whiteBuffer + symbol;
						whiteBuffer = "";
					}
				} else {
					if(isCharOneOf(symbol, ATOC_INLINE_STATEMENT_END)) {
						settings.set(configKey, currentString);
						currentString = "";
						if(symbol == ';') {
							phase = Phase::key;
							await = true;
						} else {
							parseMode = ParseMode::await;
							await = true;
						}
					} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
						whiteBuffer += symbol;
					} else {
						currentString += whiteBuffer + symbol;
						whiteBuffer = "";
					}
				}
				break;
			case ParseMode::cmd:
				if(isCharOneOf(symbol, ATOC_NEWLINE)) {
					ATOC_LOG(parser, debug) << currentString << std::endl;
					std::system(currentString.c_str());
					currentString = ""; whiteBuffer = "";
					parseMode = ParseMode::await;
					await = true;
				} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
					whiteBuffer += symbol;
				} else {
					currentString += whiteBuffer + symbol;
				}
				break;
			case ParseMode::inputDirectory:
			case ParseMode::outputDirectory:
				if(isCharOneOf(symbol, ATOC_NEWLINE)) {
					if(parseMode == ParseMode::inputDirectory)
						settings.set("input-dir", currentString);
					else
						settings.set("output-dir", currentString);
					currentString = "";
					parseMode = ParseMode::await;
					await = true;
				} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
					whiteBuffer += symbol;
				} else {
					currentString += whiteBuffer + symbol;
				}
				break;
			case ParseMode::genInput:
				if(phase == Phase::file) {
					if(symbol == '|') {
						if(currentString == "") {
							error::unexpectedSymbol(filePosition, symbol, "expected input filename");
							return false;
						} else {
							phase = Phase::var;
							currentInputIterator = input.insert({currentString,{}}).first;
							currentString = "";
							await = true;
						}
					} else if(isCharOneOf(symbol, bundled ? ATOC_INLINE_STATEMENT_END "}*" : ATOC_INLINE_STATEMENT_END "*")) {
						if(bundled) {
							if(currentString != "") {
								currentInputIterator=input.insert({currentString,{atoc::generator::generateVarName(currentString,settings)}}).first;
							}
							await = true;
							if(symbol == '}') {
								if(input.size() == 0) {
									error::statementEnd(filePosition, "expected input filenames");
									return false;
								}
								parseMode = ParseMode::genMiddle;
							} else if(symbol == '*') {
								if(currentString == "") {
									error::unexpectedSymbol(filePosition, symbol, "expected input filename", "compression operator");
									return false;
								}
								phase = Phase::compression;
							}
						} else {
							if(currentString == "") {
								error::statementEnd(filePosition, "expected input filename");
								return false;
							}
							await = true;
							if(symbol = '*') {
								currentInputIterator=input.insert({currentString,atoc::generator::OutputInformation{atoc::generator::generateVarName(currentString,settings)}}).first;
								phase = Phase::compression;
								continue;
							}
							parseMode = ParseMode::await;
							atoc::generator::generateFiles({{currentString,atoc::generator::OutputInformation{atoc::generator::generateVarName(currentString,settings)}}},{atoc::generator::generateOutputFilename(currentString,settings)},settings,forceGen);
						}
						currentString = ""; whiteBuffer = "";
					} else if(symbol == '>') {
						if(currentString == "" || bundled) {
							error::unexpectedSymbol(filePosition, symbol, "expected input filename", "output operator");
							return false;
						} else {
							input.insert({currentString, {atoc::generator::generateVarName(currentString, settings)}});
							currentString = "";
							parseMode = ParseMode::genOutput;
							
							await = true;
						}
					} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
						error::unexpectedSymbol(filePosition, symbol, "expected input filename");
						return false;
					} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
						whiteBuffer += symbol;
					} else {
						currentString += whiteBuffer + symbol;
					}
				} else if(phase == Phase::var){
					if(isCharOneOf(symbol, bundled ? ";}" : ATOC_INLINE_STATEMENT_END)) {
						if(currentString == "") {
							if(isCharOneOf(symbol, ATOC_NEWLINE)) continue;
							error::statementEnd(filePosition, "expected variable name");
							return false;
						} else {
							currentInputIterator->second = {currentString};
							currentString = "";
							if(bundled) {
								if(symbol == ';') {
									phase = Phase::file;
								} else {
									parseMode = ParseMode::genMiddle;
									await = true;
								}
							} else {
								output[0] = generator::generateOutputFilename(currentInputIterator->first, settings);
								ATOC_PARSER_GENERATE_FILES;
								output.clear();
								input.clear();
								parseMode = ParseMode::await;
							}
						}
					} else if(symbol == '>' && !bundled) {
						if(currentString == "") {
							error::statementEnd(filePosition, "expected variable name");
							return false;
						} else {
							currentInputIterator->second = {currentString};
							currentString = "";
							parseMode = ParseMode::genOutput;
							await = true;
						}
					} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
						error::unexpectedSymbol(filePosition, symbol); return false;
					} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
						whiteBuffer += symbol;
					} else {
						currentString += whiteBuffer + symbol;
					}
				} else if(phase == Phase::compression) {
					if(isCharOneOf(symbol, bundled ? ATOC_INLINE_STATEMENT_END "}" : ATOC_INLINE_STATEMENT_END ">")) {
						currentInputIterator->second.compression = atoc::generator::getCompression(currentString);
						await = true;
						currentString = ""; whiteBuffer = "";
						if(symbol == '}') {
							parseMode = ParseMode::genMiddle;
						} else if(bundled) {
							phase = Phase::file;
						} else if(symbol == '>') {
							parseMode = ParseMode::genOutput;
						} else {
							atoc::generator::generateFiles(input,{atoc::generator::generateOutputFilename(currentInputIterator->first,settings)},settings,forceGen);
							parseMode = ParseMode::await;
						}
					} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
						error::unexpectedSymbol(filePosition, symbol, "expected compression identifier", "operator");
					} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
						whiteBuffer += symbol;
					} else {
						currentString += whiteBuffer + symbol;
					}
				}
				break;
			case ParseMode::genOutput:
				if(isCharOneOf(symbol, bundled ? ATOC_INLINE_STATEMENT_END "}" : ATOC_INLINE_STATEMENT_END)) {
					if(currentString == "") {
						if(!bundled && isCharOneOf(symbol, ATOC_NEWLINE)) continue;
						if(bundled && symbol == '}' && output.size() > 0) {
							ATOC_PARSER_GENERATE_FILES;
							output.clear();
							input.clear();
							await = true;
							parseMode = ParseMode::await;
							continue;
						}
						error::statementEnd(filePosition, "expected output filename");
						return false;
					} else {
						output.push_back(currentString);
						currentString = "";
						if(bundled)
							if(symbol != '}') {
								await = true;
								phase = Phase::next;
								continue;
							} else {
								ATOC_PARSER_GENERATE_FILES;
							}
						else {
							ATOC_PARSER_GENERATE_FILES;
						}
						output.clear();
						input.clear();
						await = true;
						parseMode = ParseMode::await;
					}
				} else if(isCharOneOf(symbol, ATOC_OPERATOR)) {
					error::unexpectedSymbol(filePosition, symbol); return false;
				} else if(isCharOneOf(symbol, ATOC_WHITESPACE)) {
					whiteBuffer += symbol;
				} else {
					currentString += whiteBuffer + symbol;
				}
				break;
			default:
				break;
		}
	}
	ATOC_LOG(parser, debug) << "Print .atoccache";
	file.close();
	file.open(inputFile.parent_path().string()+"/.atoccache", std::ios_base::out | std::ios_base::trunc);
	file << ATOC_VERSION_MAJOR << "." << ATOC_VERSION_MINOR << "." << ATOC_VERSION_SUBMINOR;
	file.close();
}

bool isCharOneOf(char& needle, const char* haystack) {
	for(char* i = const_cast<char*>(haystack); *i != '\0'; i++)
		if(needle == *i) return true;
	return false;
}

void atoc::parser::error::unexpectedSymbol(FilePosition pos, char symbol, std::string expectation, std::string symbolName) {
	ATOC_LOG(parser, error) << "Unexpected " << symbolName << " '" << symbol << "' at " << pos.line << ":" << pos.column << (expectation == "" ? "" : ", ") << expectation;
}

void atoc::parser::error::statementEnd(FilePosition pos, std::string expectation) {
	ATOC_LOG(parser, error) << "Unexpected statement end at " << pos.line << ":" << pos.column << (expectation == "" ? "" : ", ") << expectation;
}
}
