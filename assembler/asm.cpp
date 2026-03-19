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
		std::cout << "Please provide the path to the file." << std::endl;
		return 0;
	}
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

		//for(int i = 0; i < s.size(); i++) std::cout << s[i].toStringDebug() << std::endl << std::endl;
		/*for(int i = 0; i < result->program_instructions.size(); i++) {
			std::cout << result->program_instructions[i]->toStringDebug() << std::endl;
		}
		std::cout << "\n";
		for(int i = 0; i < result->program_data.size(); i++) {
			std::cout << result->program_data[i]->toStringDebug() << std::endl;
		}
			*/
		std::cout << "[Symbol map entries]" << std::endl;
		for(auto const& pair : result->symmap) printf("%s -> %08X\n", pair.first.c_str(), pair.second);
		std::cout << "[					 ]" << std::endl;

		CodeGenerator* codegen = new CodeGenerator();

		std::ofstream output = codegen->makeBinaryFile("./out.tky", result);
		output.close();

		delete t;
		delete codegen;
		delete result;
		return 0;
	}
}
