#include "ExampleLexer.h"
#include <iostream>
#include <fstream>
#include <antlr4-runtime.h>

using namespace antlr4;

void processLexer(const std::string& filename) {
    std::ifstream stream;
    stream.open(filename);
    
    ANTLRInputStream input(stream);
    ExampleLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();

    std::cout << "📌 Tokens Generated:\n";
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }
}
