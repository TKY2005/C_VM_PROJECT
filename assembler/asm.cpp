#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<string>
#include<sstream>

#include<Parser/Parser.hpp>

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

	std::ifstream file(argv[1]);
	
	std::stringstream buff;
	buff << file.rdbuf();

	std::string s = buff.str();
	
	Tokenizer* t = new Tokenizer();

	std::vector<Token> tokens = t->tokenize(s);

	Parser* x = new Parser(tokens);
	
	std::unique_ptr<ParseResult> pr = x->parse();

	/*int result = x.assembleSource(argv[1], argv[2]);

	if (result == ERR_ASM_FAIL) {
		std::cout << "Assembler failed and no output file was written." << std::endl;
	}
	else if (result == ERR_ASM_WARN) {
		std::cout << 
		"The code has been successfully assembled with warnings." 
		<< std::endl;
	}*/

	//std::cout << "Assembler returned with code: " << result << std::endl;
	return 0;
}
