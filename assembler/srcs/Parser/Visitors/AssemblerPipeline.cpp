#include<any>
#include<stdexcept>

#include<Parser/Parser.hpp>
#include<ErrorHandler/ErrorHandler.hpp>
#include<Visitors/Visitors.hpp>

std::any AssemblerPipeline::assemble(ParseResult& result, AsmContext& c)
{
    SemanticAnalysisVisitor semanticAnalysis;
    semanticAnalysis.analyze(result, c);
    if (ErrorBucket::hasFailed) {
        throw std::runtime_error("semantic analysis failed");
    }

    SymbolLengthVisitor symbolLength;
    symbolLength.calculate(result, c);
    if (ErrorBucket::hasFailed) {
        throw std::runtime_error("symbol layout failed");
    }

    CodeGenerationVisitor codeGeneration;
    std::any output = codeGeneration.generate(result, c);
    if (ErrorBucket::hasFailed) {
        throw std::runtime_error("code generation failed");
    }
    return output;
}
