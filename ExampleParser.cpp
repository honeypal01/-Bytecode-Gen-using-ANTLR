#include "ExampleParser.h"
#include "ExampleLexer.h"
#include <iostream>
#include <fstream>
#include <antlr4-runtime.h>

using namespace antlr4;

void processParser(const std::string& filename) {
    std::ifstream stream;
    stream.open(filename);

    ANTLRInputStream input(stream);
    ExampleLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    ExampleParser parser(&tokens);

    ExampleParser::ProgContext* tree = parser.prog();  // Parsing

    std::cout << "📌 Parse Tree Generated!" << std::endl;
}
