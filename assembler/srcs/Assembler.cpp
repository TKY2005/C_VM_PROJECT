#include<string>
#include<vector>
#include<fstream>

#include<Assembler.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<Parser/Parser.hpp>
#include<Codegen/CodeGenerator.hpp>
#include<ErrorHandler/ErrorHandler.hpp>

std::string Assembler::filename;

int Assembler::assembleSource(std::string sourcefile, std::string destinationfile) {

    Assembler::filename = sourcefile;

    int state = ASM_SUCCESS;

    std::ifstream source(sourcefile);

    if (!source.is_open()) {
        std::cout << "Couldn't open file: '" << sourcefile << "'" << std::endl;
        return ERR_FILE_NOT_OPEN;
    }
    std::ostringstream content;
    content << source.rdbuf();

    std::string sourcecode = content.str();

    std::vector<Token> tokens = tokenizer.tokenize( tokenizer.preProcessCode(sourcecode) );
    ParseResult* result = parser.parseTokens(tokens);

    if (!ErrorBucket::getCriticalErrors().empty()) {
        std::vector<Error> err = ErrorBucket::getErrorList();
        for(int i = 0; i < err.size(); i++) {
            std::cout << err[i].errmsg() << std::endl;
        }
        return ERR_ASM_FAIL;
    }
    if (!ErrorBucket::getWarnings().empty()) {
        state = ERR_ASM_WARN;
        std::vector<Error> err = ErrorBucket::getWarnings();
        for(int i = 0; i < err.size(); i++) {
            std::cout << err[i].errmsg() << std::endl;
        }
    }

    std::cout << "[Symbol map entries]" << std::endl;
		for(auto const& pair : result->symmap) printf("%s -> %08X\n", pair.first.c_str(), pair.second);
		std::cout << "[					 ]" << std::endl;

    codegen.makeBinaryFile(destinationfile, result);
    
    delete result;
    return state;
}