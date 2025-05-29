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

int main() {  
    try {

        std::ofstream clearErr("C:/Users/kanch/Downloads/BytecodeGenerator_Project/output/errors.txt", std::ios::trunc);
        clearErr.close();


        // Step 1: Input file read karna
        std::ifstream inputFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\build\\input.txt");
        if (!inputFile) {
            std::cerr << "❌ Error: input.txt file open nahi ho paayi! Make sure input.txt build folder ke andar ho." << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string code = buffer.str();

        // Step 2: Tokenization - Lexer phase
        ANTLRInputStream input(code);
        ExampleLexer lexer(&input);
        CommonTokenStream tokens(&lexer);

        std::ofstream tokenFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\token_list.txt");
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
        std::ofstream runOutput("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\run_output.txt", std::ios::trunc);

        // 🧨 Error found
        if (errorListener.hasErrors()) {
            std::cout << "\n⚠️ Multiple syntax errors found in input:\n";
            runOutput << "\n⚠️ Multiple syntax errors found in input:\n";

            errorListener.printErrorsToConsole();
            errorListener.writeErrorsToFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\errors.txt");

            for (const auto& err : errorListener.errorMessages) {
                runOutput << err << std::endl;
            }

            // ❌ Dummy parse tree to avoid TreePrinter error
            std::ofstream pt("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\tree.txt");
            pt << "❌ Invalid input! No textual parse tree generated.\n";
            pt << "➡️ Tip: Input should start with '(prog ... )'\n";
            pt << "🛠️ Fix: Run BytecodeGenerator.exe with valid syntax.\n";
            pt.close();

            std::cout << "\n🚫 Skipping bytecode generation due to syntax errors.\n";
            runOutput << "\n🚫 Skipping bytecode generation due to syntax errors.\n";

            runOutput.close();
            return 1;
        }else {
            std::cout << "✅ No syntax errors found.\n";
            // ⏩ Continue to parse tree + bytecode...
        }


        std::ofstream treeOut("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\tree.txt");
        treeOut << tree->toStringTree(&parser);
        treeOut.close();

        std::cout << "\n🔸 PARSE TREE (Textual):\n" << tree->toStringTree(&parser) << std::endl;

        std::string parseTreeStr = tree->toStringTree(&parser);

        // Step 4: Java TreePrinter se DOT generate
        std::cout << "\n🖼️ Parse Tree Diagram Generate kar rahe hain...\n";
        int result = system(
            "java -cp \"../grammar/antlr4-4.9.3-complete.jar;../tools\" TreePrinter ../output/tree.txt ../output/parse_tree.dot"
        );
        if (result != 0) {
            std::cerr << "❌ TreePrinter Java program run nahi hua! Path ya tree.txt check karo.\n";
            return 1;
        }

        // Step 5: Graphviz se PNG generate
        result = system("dot -Tpng ../output/parse_tree.dot -o ../output/parse_tree.png");
        if (result != 0) {
            std::cerr << "❌ Graphviz 'dot' command failed! Graphviz install aur PATH check karo.\n";
            return 1;
        }

        // Step 6: Image auto open karna
        system("start ../output/parse_tree.png");

        // 👇 STEP: Semantic Checking
        SemanticChecker semaChecker;
        tree::ParseTreeWalker::DEFAULT.walk(&semaChecker, tree);
        // semaChecker.writeSymbolTableToFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\symbol_table.txt");
        semaChecker.writeAllToFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\symbol_table.txt");


        if (semaChecker.hasErrors()) {
            
            semaChecker.writeErrorsToFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\errors.txt");

            std::cout << "\n🚫 Semantic errors detected. Bytecode generation skipped.\n";
            runOutput << "\n🚫 Semantic errors detected. Bytecode generation skipped.\n";
            runOutput.close();
            return 1;
        } else {
            std::cout << "✅ No semantic errors found.\n";
        }

        std::cout << "Total semantic errors: " << semaChecker.getSemanticErrors().size() << std::endl;


        // Step 7: Bytecode Generation
        BytecodeGen listener;
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

        std::ofstream bytecodeFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\bytecode_output.txt");
        std::cout << "\n✅ GENERATED BYTECODE:\n";
        bytecodeFile << "GENERATED BYTECODE:" << std::endl;

        for (const std::string &instr : listener.bytecode) {
            std::cout << instr << std::endl;
            bytecodeFile << instr << std::endl;
        }
        bytecodeFile.close();

        // 🖨️ Write final output from print statements
        listener.writeFinalOutputToFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\final_output.txt");

        std::ofstream fout("output/final_output.txt");
        for (const std::string &line : listener.finalOutput) {
            fout << line << std::endl;
        }
        fout.close();

        // Final Stats
        int tokenCount = tokens.getTokens().size();
        int parseTreeNodeCount = std::count(parseTreeStr.begin(), parseTreeStr.end(), '(');
        int bytecodeCount = listener.bytecode.size();

        std::cout << "\n🧾 Token Count: " << tokenCount << std::endl;
        std::cout << "🌳 Parse Tree Nodes: " << parseTreeNodeCount << std::endl;
        std::cout << "💾 Bytecode Instructions: " << bytecodeCount << std::endl;

        // Log file
        std::ofstream logFile("C:\\Users\\kanch\\Downloads\\BytecodeGenerator_Project\\output\\run_log.txt");
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

        // Export outputs
        fs::create_directories("../exports");
        try {
            fs::copy_file("../output/token_list.txt", "../exports/token_list.txt", fs::copy_options::overwrite_existing);
            fs::copy_file("../output/bytecode_output.txt", "../exports/bytecode_output.txt", fs::copy_options::overwrite_existing);
            fs::copy_file("../output/parse_tree.png", "../exports/parse_tree.png", fs::copy_options::overwrite_existing);
            fs::copy_file("../output/run_output.txt", "../exports/run_output.txt", fs::copy_options::overwrite_existing);
        } catch (fs::filesystem_error& e) { 
            std::cerr << "Export error: " << e.what() << std::endl;
        }

        runOutput << "\n✅ Execution completed successfully.\n";
        runOutput.close();


    } catch (const std::exception& e) {
        std::ofstream errorFile("../output/errors.txt");
        errorFile << "❌ Syntax or Runtime Error: " << e.what() << std::endl;
        errorFile.close();
        std::cerr << "❌ Syntax or Runtime Error: " << e.what() << std::endl;
    }

    return 0;
}