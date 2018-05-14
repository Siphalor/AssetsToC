#include "generator.hpp"

namespace atoc::generator {

bool generateFiles(std::map<std::string,std::string> input, std::vector<std::string> output, Settings& settings, bool force) {
	BOOST_LOG_TRIVIAL(debug) << output[0];
	if(input.size() == 0 || output.size() == 0) {
		BOOST_LOG_TRIVIAL(error) << "Internal error: At least one input or output files must be specified";
		return false;
	}
	std::string outputDir = settings.getOutputDirectory() + std::string("/") + settings.get<std::string>("output-dir");
	std::string firstOutput = outputDir + "/" + output[0];
	boost::filesystem::path inputFile;
	if(!force && boost::filesystem::exists(boost::filesystem::status(boost::filesystem::path(firstOutput.c_str())))) {
		std::time_t outputTime = boost::filesystem::last_write_time(boost::filesystem::path(firstOutput.c_str()));
		bool shouldWrite = false;
		for(auto it : input) {
			inputFile = boost::filesystem::path(it.first);
			if(boost::filesystem::is_directory(inputFile) && settings.get<bool>("force-folder-rebuild")) {
				shouldWrite = true;
				break;
			}
			if(boost::filesystem::exists(boost::filesystem::status(inputFile)) && boost::filesystem::last_write_time(inputFile) > outputTime) {
				shouldWrite = true;
				break;
			}
		}
		if(!shouldWrite) return true;
	}
	boost::filesystem::create_directories(boost::filesystem::path(firstOutput).parent_path());
	std::ofstream file(firstOutput, std::ios_base::trunc | std::ios_base::out);
	std::string inputDirectory = settings.getInputDirectory() + "/" + settings.get<std::string>("input-dir") + "/";
	for(auto it : input) {
		inputFile = boost::filesystem::path(inputDirectory + it.first);
		if(!boost::filesystem::exists(boost::filesystem::status(inputFile))) continue;
		if(boost::filesystem::is_directory(boost::filesystem::status(inputFile))) {
			boost::filesystem::recursive_directory_iterator directory(inputFile, boost::filesystem::symlink_option::recurse), end;
			while(directory != end) {
				if(boost::filesystem::is_regular_file(directory->status())) {
					processFile(directory->path(), generateVarName(directory->path().string().substr(inputDirectory.size()-1), settings), file, settings);
				}
				directory++;
			}
		} else {
			processFile(inputFile, it.second, file, settings);
		}
	}
	file.close();

	for(auto& out : output) {
		if(out == output[0]) continue;
		boost::filesystem::create_directories(boost::filesystem::path(outputDir+"/"+out).parent_path());
		boost::filesystem::copy_file(firstOutput, outputDir+"/"+out, boost::filesystem::copy_option::overwrite_if_exists);
	}
}

bool processFile(boost::filesystem::path file, std::string variableName, std::ofstream& output, Settings& settings) {
	Settings::Setting::number_type bufferSize = settings.get<Settings::Setting::number_type>("input-buffer-size");
	unsigned fileSize = boost::filesystem::file_size(file);
	char content[bufferSize];
	output << settings.get<std::string>("array-type") << "[" << std::to_string(fileSize) << "] " << variableName << " = {";
	std::ifstream inputFile(file.string());
	unsigned blocks = fileSize / bufferSize;
	const bool useHex = settings.get<bool>("use-hex");
	for(unsigned b = 0; b <= blocks; b++) {
		inputFile.read(content, b < blocks ? bufferSize : fileSize % bufferSize);
		for(unsigned i = 0; i < (b < blocks ? bufferSize : fileSize % bufferSize); i++) {
			if(useHex)
				output << "0x" << std::hex << (unsigned short)(unsigned char)content[i] << (i >= fileSize - 1 ? "" : ",");
			else output << (unsigned short)(unsigned char)content[i] << (i >= fileSize - 1 ? "" : ",");
		}
	}
	inputFile.close();
	output << "};\n";
	if(settings.get<bool>("array-size"))
		output << settings.get<std::string>("array-size-type") << " " << variableName + settings.get<std::string>("array-size-suffix") << " = " << std::to_string(fileSize) << ";\n";
}

std::string generateVarName(std::string input, Settings& settings) {
	if(!settings.get<bool>("array-name-include-folders")) input = boost::filesystem::path(input).filename().string();
	std::vector<std::string> parts;
	boost::algorithm::split(parts, input, boost::algorithm::is_any_of("/\\."), boost::algorithm::token_compress_on);
	parts.erase(std::remove(parts.begin(), parts.end(), ""), parts.end());
	if(settings.get<bool>("array-name-camelcase"))
		for(auto& part : parts) {
			part[0] = toupper(part[0]);
		}
	if(!settings.get<bool>("array-name-capital")) parts[0][0] = tolower(parts[0][0]);
	else parts[0][0] = toupper(parts[0][0]);
	if(!settings.get<bool>("array-name-include-extension") && boost::filesystem::path(input).has_extension()) parts.pop_back();
	return boost::algorithm::join(parts, settings.get<std::string>("array-name-delimiter"));
}

std::string generateOutputFilename(std::string input, Settings& settings) {
	boost::filesystem::path path(input);
	if(!settings.get<bool>("file-include-folders")) input = path.filename().string();
	std::vector<std::string> parts;
	boost::algorithm::split(parts, input, boost::algorithm::is_any_of("/\\."), boost::algorithm::token_compress_on);
	parts.erase(std::remove(parts.begin(), parts.end(), ""), parts.end());
	if(settings.get<bool>("file-camelcase"))
		for(auto& part : parts) {
			part[0] = toupper(part[0]);
		}
	if(!settings.get<bool>("file-capital")) parts[0][0] = tolower(parts[0][0]);
	else parts[0][0] = toupper(parts[0][0]);
	if(settings.get<bool>("copy-folder-system")) parts.insert(parts.begin(), path.parent_path().string());
	if(!settings.get<bool>("file-include-extension") && path.has_extension()) parts.pop_back();
	return boost::algorithm::join(parts, settings.get<std::string>("file-delimiter")) + settings.get<std::string>("file-extension");
}

const std::string Settings::Setting::toString() const {
	switch(type) {
		case Type::string: return *(std::string*)value; break;
		case Type::number: return std::to_string(*(Settings::Setting::number_type*)value); break;
		case Type::boolean: return std::to_string(*(bool*)value); break;
		case Type::fault: return "<fault>"; break;
	}
	return "<null>";
}

Settings::~Settings() {
	while(values.size() > 0) values.erase(values.begin());
}

const std::map<std::string, Settings::Setting> Settings::settings = {
	{"input-dir", {"Sets the input root directory",Settings::Setting::Type::string,new std::string("")}},
	{"output-dir", {"Sets the output root directory",Settings::Setting::Type::string,new std::string("")}},
	{"force-folder-rebuild", {"Forces the generator to rebuild folder entries every time",Settings::Setting::Type::boolean,new bool(true)}},
	{"input-buffer-size", {"Sets the size of the asset input buffer",Settings::Setting::Type::number,new Settings::Setting::number_type(1024u),Settings::Setting::nonzero}},
	{"array-name-delimiter", {"Sets the delimiter for the automatic output variable naming",Settings::Setting::Type::string,new std::string("")}},
	{"array-name-camelcase", {"Sets the camel case flag for the automatic output variable naming",Settings::Setting::Type::boolean,new bool(true)}},
	{"array-name-include-folders", {"Sets wether folders are integrated in the automatic output variable naming (begins at the current root directory)",Settings::Setting::Type::boolean,new bool(true)}},
	{"array-name-capital", {"Sets wether to use a captial letter at the generated name beginning",Settings::Setting::boolean,new bool(false)}},
	{"array-type", {"Sets the type of the output arrays",Settings::Setting::Type::string,new std::string("const unsigned char"),Settings::Setting::notnull}},
	{"array-size", {"Sets wether the output array size ist stored in its own variable",Settings::Setting::Type::boolean,new bool(true)}},
	{"array-size-type", {"Sets the type of the output array size variable",Settings::Setting::Type::string,new std::string("const unsigned"),Settings::Setting::notnull}},
	{"array-size-suffix", {"Sets the suffix for the output array size variable",Settings::Setting::Type::string,new std::string("_size")}},
	{"use-hex", {"Sets whether the output array uses hex declaration",Settings::Setting::Type::boolean,new bool(true)}},
	{"file-extension", {"Sets the extension of the output files",Settings::Setting::Type::string,new std::string(".hpp")}},
	{"file-delimiter", {"Sets the delimiter for the automatic file naming",Settings::Setting::Type::string,new std::string("")}},
	{"file-camelcase", {"Sets the camel case flag for the automatic file naming",Settings::Setting::Type::boolean,new bool(false)}},
	{"file-capital", {"Sets wether to use a captial letter at the generated filename beginning",Settings::Setting::boolean,new bool(false)}},
	{"file-include-folders", {"Sets wether folders are integrated in the automatic file naming (begins at the current root directory)",Settings::Setting::Type::boolean,new bool(false)}},
	{"copy-folder-system", {"Sets wether the same folder names are used when automatically generating output file names (begins at the current root directory)",Settings::Setting::Type::boolean,new bool(false)}},
};

void Settings::printHelp() {
	for(auto& setting : settings) {
		std::cout << "\t" << setting.first << " (";
		if(setting.second.type==Setting::Type::string) std::cout << "string";
		else if(setting.second.type==Setting::Type::boolean) std::cout << "boolean";
		else if(setting.second.type==Setting::Type::number) std::cout << "number";
		std::cout << "): " << setting.second.description << std::endl;
		std::cout << "\t\tdefault: " << setting.second.toString() << std::endl;
	}
}

bool Settings::set(std::string key, std::string value) {
	Setting::Type type = Setting::Type::string;
	bool native = (settings.find(key) != settings.end());
	if(native) {
		type = settings.at(key).type;
		if(settings.at(key).flags & Setting::notnull && value == "") {
			BOOST_LOG_TRIVIAL(error) << "Cannot set config \"" << key << "\" to null";
			return false;
		}
	}
	switch(type) {
		case Setting::Type::string:
			delete (std::string*)values[key];
			values[key] = (void*) new std::string(value);
			break;
		case Setting::Type::number:
			try {
				long val = std::stol(value);
				if(native && !val) {
					BOOST_LOG_TRIVIAL(error) << "Config \"" << key << "\" must be at least 1";
					return false;
				}
				if(val > 8388608) {
					BOOST_LOG_TRIVIAL(warning) << "Config for \"" << key << "\" is too high, it must be at most 838860";
					return false;
				} else if(val < 0) {
					BOOST_LOG_TRIVIAL(warning) << "Config for \"" << key << "\" is too small, it must be at least 0";
					return false;
				}
				delete (Settings::Setting::number_type*)values[key];
				values[key] = (void*) new Settings::Setting::number_type(val);
				BOOST_LOG_TRIVIAL(debug) << val;
			} catch (std::exception e) {
				BOOST_LOG_TRIVIAL(error) << "Unable to convert \"" << value << "\" to config number for \"" << key << "\"";
				if(native) return false;
				values[key] = (void*) new Settings::Setting::number_type(0u);
				return false;
			}
			break;
		case Setting::Type::boolean: {
			delete (bool*)values[key];
			bool boolean = true;
			if(value == "0" || value == "false" || value == "") boolean = false;
			values[key] = (void*) new bool(boolean);
			break; }
		default:
			break;
	}
	return true;
}

template<typename T> T Settings::get(std::string key) {
	if(values.find(key) == values.end()) {
		if(settings.find(key) == settings.end()) {
			return T();
		} else {
			return *((T*)settings.at(key).value);
		}
	} else {
		return *((T*)values[key]);
	}
}

Settings::Setting::Type Settings::getType(std::string key) {
	if(settings.find(key) != settings.end()) return settings.at(key).type;
	if(values.find(key) == values.end()) return Setting::Type::fault;
	else return Setting::Type::string;
}

void Settings::deleteEntry(std::string key) {
	switch(getType(key)) {
		case Setting::Type::string:
			delete (std::string*)values[key];
			break;
		case Setting::Type::number:
			delete (Settings::Setting::number_type*)values[key];
			break;
		case Setting::Type::boolean:
			delete (bool*)values[key];
			break;
		default:
			BOOST_LOG_TRIVIAL(error) << "Could not delete config entry: No such config key!";
			return;
			break;
	}
	values.erase(values.find(key));
}

void Settings::setIO(std::string inputDirectory_, std::string outputDirectory_) {
	inputDirectory = inputDirectory_;
	outputDirectory = outputDirectory_;
}

}
