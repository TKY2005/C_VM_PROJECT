#include<Assembler.hpp>
#include<Parser/Parser.hpp>
#include<Visitors/Visitors.hpp>

#include<fstream>
#include<iomanip>
#include<iostream>
#include<iterator>
#include<sstream>
#include<stdexcept>
#include<string>
#include<utility>
#include<vector>
#include<ErrorHandler/ErrorHandler.hpp>

std::string Assembler::filename;

int Assembler::assembleSource(
    std::string sourcefile,
    std::string destinationfile,
    bool printListing,
    uint64_t maximumImageSize,
    std::map<std::string, uint32_t> symbols
)
{
    ErrorBucket::clear();
    filename = sourcefile;

    std::ifstream source(sourcefile);
    if (!source.is_open()) {
        std::cerr << "Unable to open source file: " << sourcefile << '\n';
        return ERR_FILE_NOT_OPEN;
    }

    std::string sourceText(
        (std::istreambuf_iterator<char>(source)),
        std::istreambuf_iterator<char>()
    );
    if (sourceText.size() >= 3 &&
        static_cast<uint8_t>(sourceText[0]) == 0xef &&
        static_cast<uint8_t>(sourceText[1]) == 0xbb &&
        static_cast<uint8_t>(sourceText[2]) == 0xbf) {
        sourceText.erase(0, 3);
    }
    std::vector<std::string> sourceLines;
    std::stringstream sourceStream(sourceText);
    std::string sourceLine;
    while (std::getline(sourceStream, sourceLine)) {
        if (!sourceLine.empty() && sourceLine.back() == '\r') sourceLine.pop_back();
        if (sourceLines.empty() && sourceLine.size() >= 3 &&
            static_cast<uint8_t>(sourceLine[0]) == 0xef &&
            static_cast<uint8_t>(sourceLine[1]) == 0xbb &&
            static_cast<uint8_t>(sourceLine[2]) == 0xbf) {
            sourceLine.erase(0, 3);
        }
        sourceLines.push_back(sourceLine);
    }

    try {
        std::vector<Token> tokens = tokenizer.tokenize(tokenizer.preProcessCode(sourceText));
        if (ErrorBucket::hasFailed) {
            ErrorBucket::reportErrors(std::cerr);
            return ERR_ASM_FAIL;
        }
        Parser parser(std::move(tokens), sourceLines, sourcefile);
        std::unique_ptr<ParseResult> result = parser.parse();
        if (!result) throw std::runtime_error("parser returned no result");
        if (ErrorBucket::hasFailed) {
            ErrorBucket::reportErrors(std::cerr);
            return ERR_ASM_FAIL;
        }

        AsmContext context;
        context.sourceFile = sourcefile;
        context.symMap = std::move(symbols);
        AssemblerPipeline pipeline;
        std::vector<uint8_t> output =
            std::any_cast<std::vector<uint8_t>>(pipeline.assemble(*result, context));
        if (output.size() > maximumImageSize) {
            std::cerr << "Assembled image size (" << output.size()
                      << " bytes) exceeds the maximum of " << maximumImageSize
                      << " bytes; output file was not written.\n";
            return ERR_ASM_FAIL;
        }

        std::ofstream destination(destinationfile, std::ios::binary);
        if (!destination.is_open()) {
            std::cerr << "Unable to open output file: " << destinationfile << '\n';
            return ERR_ASM_FAIL;
        }
        if (!output.empty()) {
            destination.write(
                reinterpret_cast<const char*>(output.data()),
                static_cast<std::streamsize>(output.size())
            );
        }
        if (!destination) {
            std::cerr << "Failed while writing output file: " << destinationfile << '\n';
            return ERR_ASM_FAIL;
        }
        if (printListing) {
            for (const auto& node : result->parsedLines) {
                std::cout << node->sourceLine << " ->";
                const auto addresses = context.nodeAddresses.find(node.get());
                const auto lengths = context.nodeLengths.find(node.get());
                if (addresses != context.nodeAddresses.end() && !addresses->second.empty() &&
                    lengths != context.nodeLengths.end() && !lengths->second.empty()) {
                    const uint64_t address = addresses->second.front();
                    const uint64_t length = lengths->second.front();
                    if (address >= context.generatedBaseAddress &&
                        address - context.generatedBaseAddress + length <= context.generatedBytes.size()) {
                        for (uint64_t offset = 0; offset < length; ++offset) {
                            std::cout << " 0x" << std::uppercase << std::hex << std::setw(2)
                                      << std::setfill('0')
                                      << static_cast<unsigned int>(context.generatedBytes[
                                             static_cast<size_t>(address - context.generatedBaseAddress + offset)]);
                        }
                        std::cout << std::dec << std::setfill(' ');
                    }
                }
                std::cout << '\n';
            }
        }
    }
    catch (const std::exception& error) {
        if (ErrorBucket::hasFailed) ErrorBucket::reportErrors(std::cerr);
        else std::cerr << "Assembly failed: " << error.what() << '\n';
        return ERR_ASM_FAIL;
    }

    return ASM_SUCCESS;
}