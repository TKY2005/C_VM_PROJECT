#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<string>
#include<sstream>
#include<any>

#include<Parser/Parser.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<Visitors/Visitors.hpp>

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
	
	Tokenizer t = Tokenizer();

	std::vector<Token> tokens = t.tokenize(t.preProcessCode(s));

	Parser p = Parser(tokens);
	
	std::unique_ptr<ParseResult> pr = p.parse();

	PrintVisitor v;

	AsmContext& c = *(std::unique_ptr<AsmContext>(new AsmContext()));

	for(int i = 0; i < pr.get()->parsedLines.size(); i++) {
		std::cout << std::any_cast<std::string>(v.traverse(*(pr.get()->parsedLines[i]), c)) << "\n\n";
	}

	return 0;
}
