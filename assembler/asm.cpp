#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<string>
#include<sstream>

#include "./headers/LexicalAnalyzer/Tokenizer.hpp"
#include "./headers/Parser/Parser.hpp"
#include "./headers/Codegen/CodeGenerator.hpp"
#include "../shared/utils/ds.h"

int main(int argc, char** argv) {
	if (argv[1] == NULL) {
		std::cout << "Please provide the path to the source file." << std::endl;
		return 0;
	}
	std::string outputFileName;
	if (argv[2] == NULL) {
		outputFileName = "./out.tky";
	}
	else outputFileName = argv[2];

	std::ifstream sourcefile(argv[1]);

	if (!sourcefile.is_open()) {
		std::cout << "Unable to open file: '" << argv[1] << "'" << std::endl;
		return 0;
	}
	else{
		std::ostringstream content;
		content << sourcefile.rdbuf();
		std::string source = content.str();

		Tokenizer* t = new Tokenizer();
		std::string code = t->preProcessCode(source);
		std::vector<Token> s = t->tokenize(code);

		Parser p;

		ParseResult* result = p.parseTokens(s);

		std::cout << "[Symbol map entries]" << std::endl;
		for(auto const& pair : result->symmap) printf("%s -> %08X\n", pair.first.c_str(), pair.second);
		std::cout << "[					 ]" << std::endl;

		CodeGenerator* codegen = new CodeGenerator();

		std::ofstream output = codegen->makeBinaryFile(outputFileName, result);
		output.close();

		delete t;
		delete codegen;
		delete result;
		return 0;
	}
}
