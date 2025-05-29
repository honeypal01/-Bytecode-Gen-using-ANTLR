#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>     // for system()
#include <algorithm>   // for std::count
#include <filesystem>  // for file copy/export
#include "ExampleLexer.h"
#include "ExampleParser.h"
#include "BytecodeGen.h"
#include "antlr4-runtime.h"
#include "CustomErrorListener.h"   // 🔥 Add this line
#include "SemanticChecker.h"

using namespace antlr4;
namespace fs = std::filesystem;

void processFile(const std::string &inputFileName, const std::string &outputPrefix) {
    try {
        std::cout << "\nProcessing file: " << inputFileName << std::endl;

        std::ofstream clearErr(outputPrefix + "_errors.txt", std::ios::trunc);
        clearErr.close();

        // Step 1: Input file read karna
        std::ifstream inputFile(inputFileName);
        if (!inputFile) {
            std::cerr << "❌ Error: " << inputFileName << " file open nahi ho paayi!" << std::endl;
            return;
        }

        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string code = buffer.str();

        // Step 2: Tokenization - Lexer phase
        ANTLRInputStream input(code);
        ExampleLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        std::ofstream tokenFile(outputPrefix + "_token_list.txt");
        std::cout << "\n🔹 TOKENS:\n";
        tokens.fill();

        for (auto token : tokens.getTokens()) {
            std::string line = "Token: " + token->getText() +
                               " | Type: " + lexer.getVocabulary().getSymbolicName(token->getType());
            std::cout << line << std::endl;
            tokenFile << line << std::endl;
        }
        tokenFile.close();

        // Step 3: Parsing with Error Listener
        ExampleParser parser(&tokens);
        CustomErrorListener errorListener;

        parser.removeErrorListeners(); 
        parser.addErrorListener(&errorListener);

        // ✨ Multiple errors allowed
        parser.setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());

        // Parsing start
        tree::ParseTree *tree = nullptr;

        try {
            tree = parser.prog();  // 👈 parser rule "prog"
        } catch (const std::exception &e) {
            std::cerr << "Exception caught during parsing: " << e.what() << "\n";
        }

        // 🧾 Open run_output.txt to log
        std::ofstream runOutput(outputPrefix + "_run_output.txt", std::ios::trunc);

        // 🧨 Error found
        if (errorListener.hasErrors()) {
            std::cout << "\n⚠️ Multiple syntax errors found in input:\n";
            runOutput << "\n⚠️ Multiple syntax errors found in input:\n";

            errorListener.printErrorsToConsole();
            errorListener.writeErrorsToFile(outputPrefix + "_errors.txt");

            for (const auto& err : errorListener.errorMessages) {
                runOutput << err << std::endl;
            }

            // ❌ Dummy parse tree to avoid TreePrinter error
            std::ofstream pt(outputPrefix + "_tree.txt");
            pt << "❌ Invalid input! No textual parse tree generated.\n";
            pt << "➡️ Tip: Input should start with '(prog ... )'\n";
            pt << "🛠️ Fix: Run BytecodeGenerator.exe with valid syntax.\n";
            pt.close();

            std::cout << "\n🚫 Skipping bytecode generation due to syntax errors.\n";
            runOutput << "\n🚫 Skipping bytecode generation due to syntax errors.\n";

            runOutput.close();
            return;
        } else {
            std::cout << "✅ No syntax errors found.\n";
        }

        // Step 4: Output the parse tree to file
        std::ofstream treeOut(outputPrefix + "_tree.txt");
        treeOut << tree->toStringTree(&parser);
        treeOut.close();

        std::cout << "\n🔸 PARSE TREE (Textual):\n" << tree->toStringTree(&parser) << std::endl;

        std::string parseTreeStr = tree->toStringTree(&parser);

        // Step 5: Generate Parse Tree Diagram using TreePrinter
        std::cout << "\n🖼️ Parse Tree Diagram Generate kar rahe hain...\n";
        int result = system(
            ("java -cp \"../grammar/antlr4-4.9.3-complete.jar;../tools\" TreePrinter " +
             outputPrefix + "_tree.txt " + outputPrefix + "_parse_tree.dot").c_str()
        );
        if (result != 0) {
            std::cerr << "❌ TreePrinter Java program run nahi hua! Path ya tree.txt check karo.\n";
            return;
        }

        // Step 6: Graphviz se PNG generate
        result = system(("dot -Tpng " + outputPrefix + "_parse_tree.dot -o " + outputPrefix + "_parse_tree.png").c_str());
        if (result != 0) {
            std::cerr << "❌ Graphviz 'dot' command failed! Graphviz install aur PATH check karo.\n";
            return;
        }

        // Step 7: Open image
        system(("start " + outputPrefix + "_parse_tree.png").c_str());

        // Semantic checking step
        SemanticChecker semaChecker;
        tree::ParseTreeWalker::DEFAULT.walk(&semaChecker, tree);

        if (semaChecker.hasErrors()) {
            std::cout << "\n🚫 Semantic errors detected. Bytecode generation skipped.\n";
            runOutput << "\n🚫 Semantic errors detected. Bytecode generation skipped.\n";
            runOutput.close();
            return;
        } else {
            std::cout << "✅ No semantic errors found.\n";
        }

        // Step 8: Bytecode Generation
        BytecodeGen listener;
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

        std::ofstream bytecodeFile(outputPrefix + "_bytecode_output.txt");
        std::cout << "\n✅ GENERATED BYTECODE:\n";
        bytecodeFile << "GENERATED BYTECODE:" << std::endl;

        for (const std::string &instr : listener.bytecode) {
            std::cout << instr << std::endl;
            bytecodeFile << instr << std::endl;
        }
        bytecodeFile.close();

        // Final Stats
        int tokenCount = tokens.getTokens().size();
        int parseTreeNodeCount = std::count(parseTreeStr.begin(), parseTreeStr.end(), '(');
        int bytecodeCount = listener.bytecode.size();

        std::cout << "\n🧾 Token Count: " << tokenCount << std::endl;
        std::cout << "🌳 Parse Tree Nodes: " << parseTreeNodeCount << std::endl;
        std::cout << "💾 Bytecode Instructions: " << bytecodeCount << std::endl;

        // Log file
        std::ofstream logFile(outputPrefix + "_run_log.txt");
        if (logFile.is_open()) {
            logFile << "=============================================\n";
            logFile << "🕒 Run Timestamp: ";
            time_t now = time(0);
            logFile << ctime(&now);
            logFile << "\n📂 Input File Content:\n" << code;
            logFile << "\n🧾 Token Count: " << tokenCount;
            logFile << "\n🌳 Parse Tree Nodes: " << parseTreeNodeCount;
            logFile << "\n💾 Bytecode Instructions: " << bytecodeCount;
            logFile << "\n=============================================\n";
            logFile.close();
        } else {
            std::cerr << "❌ run_log.txt file likhne me problem aayi!" << std::endl;
        }

        runOutput << "\n✅ Execution completed successfully for " << inputFileName << ".\n";
        runOutput.close();


    }   catch (const std::exception& e) {
        std::ofstream errorFile("../output_multi/errors.txt");
        errorFile << "❌ Syntax or Runtime Error: " << e.what() << std::endl;
        errorFile.close();
        std::cerr << "❌ Syntax or Runtime Error: " << e.what() << std::endl;
    }
}

int main() {
    // Process multiple input files
    std::vector<std::string> inputFiles = {"../build_multi/inputs/input1.txt", "../build_multi/inputs/input2.txt", "../build_multi/inputs/input3.txt"};
    for (const std::string& fileName : inputFiles) {
        std::string outputPrefix = "../output_multi/" + fileName.substr(fileName.find_last_of("/\\") + 1);
        outputPrefix = outputPrefix.substr(0, outputPrefix.find_last_of("."));
        processFile(fileName, outputPrefix);
    }

    return 0;
}
