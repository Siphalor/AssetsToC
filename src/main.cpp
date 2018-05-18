#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "parser.hpp"

int main(int argc, char* argv[]) {
	boost::program_options::options_description optionDesc("Allowed Arguments");
	optionDesc.add_options()
		("help", "produce this help message")
		("help-config", "prints all available configurations for use in .atoc files")
		("version", "get the current version number")
		("credits", "displays credits for this beautiful work of coding")
		("input-dir", boost::program_options::value<std::string>(), "the input root directory with the .atoc file")
		("output-dir", boost::program_options::value<std::string>(), "the root directory for the generated c files");
	boost::program_options::positional_options_description positionalOptionDesc;
	positionalOptionDesc.add("input-dir", 1);
	positionalOptionDesc.add("output-dir", 1);

	boost::program_options::variables_map programOptions;
	try {
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(optionDesc).positional(positionalOptionDesc).run(), programOptions);
	} catch (std::exception e) {
		std::cout << "Unknown argument" << std::endl;
		return true;
	}
	boost::program_options::notify(programOptions);
	
	if(programOptions.count("help")) {
		std::cout << ATOC_APP_NAME << " " << ATOC_VERSION_MAJOR << "." << ATOC_VERSION_MINOR << "." << ATOC_VERSION_SUBMINOR << std::endl;
		std::cout << "atoc [<input-dir>] [<output-dir>]" << std::endl;
		std::cout << optionDesc << std::endl;
		return false;
	} if(programOptions.count("help-config")) {
		std::cout << "All available configuration keys:" << std::endl;
		atoc::generator::Settings::printHelp();
		return false;
	} if(programOptions.count("version")) {
		std::cout << ATOC_APP_NAME << " " << ATOC_VERSION_MAJOR << "." << ATOC_VERSION_MINOR << "." << ATOC_VERSION_SUBMINOR << std::endl;
		return false;
	} if(programOptions.count("credits")) {
		std::cout << ATOC_APP_NAME << " " << ATOC_VERSION_MAJOR << "." << ATOC_VERSION_MINOR << "." << ATOC_VERSION_SUBMINOR << std::endl;
		std::cout << "Made by Siphalor\tFor now only me )':" << std::endl;
		return false;
	}

	std::string inputDir, outputDir;
	
	if(programOptions.count("input-dir")) {
		inputDir = programOptions["input-dir"].as<std::string>();
		if(!boost::filesystem::exists(boost::filesystem::status(boost::filesystem::path(inputDir)))) {
			std::cout << "The given input directory doesn't exist! " << std::endl;
			return 1;
		}
	} else {
		inputDir = boost::filesystem::current_path().string();
	}
	boost::filesystem::path inputPath(inputDir);
	if(!boost::filesystem::is_regular_file(inputPath))
 		inputPath += "/.atoc";
	if(programOptions.count("output-dir")) {
		outputDir = programOptions["output-dir"].as<std::string>();
		if(!boost::filesystem::exists(boost::filesystem::status(boost::filesystem::path(outputDir)))) {
			std::cout << "The given output directory doesn't exist! " << std::endl;
			return 1;
		}
	} else {
		outputDir = inputPath.parent_path().string();
	}
	
	atoc::parser::parseFile(inputPath, boost::filesystem::path(outputDir));

	return false;
}
