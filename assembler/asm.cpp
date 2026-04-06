#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<string>
#include<sstream>

#include<Assembler.hpp>

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

	Assembler x;

	int result = x.assembleSource(argv[1], argv[2]);

	if (result == ERR_ASM_FAIL) {
		std::cout << "Assembler failed and no output file was written." << std::endl;
	}
	else if (result == ERR_ASM_WARN) {
		std::cout << 
		"The code has been successfully assembled with warnings." 
		<< std::endl;
	}

	std::cout << "Assembler returned with code: " << result << std::endl;
	return 0;
}
