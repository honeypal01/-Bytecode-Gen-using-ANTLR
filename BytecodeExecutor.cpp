// BytecodeExecutor.cpp
#include "BytecodeExecutor.h"
#include <iostream>
#include <fstream>
#include <stack>

void BytecodeExecutor::execute(const std::vector<std::string>& bytecode, const std::string& outputPath) {
    std::stack<int> evalStack;
    std::ofstream outFile(outputPath, std::ios::trunc);

    for (const auto& line : bytecode) {
        if (line.find("PUSH") == 0) {
            int value = std::stoi(line.substr(5));  // naive parse
            evalStack.push(value);
        } else if (line == "ADD") {
            int b = evalStack.top(); evalStack.pop();
            int a = evalStack.top(); evalStack.pop();
            evalStack.push(a + b);
        } else if (line == "SUB") {
            int b = evalStack.top(); evalStack.pop();
            int a = evalStack.top(); evalStack.pop();
            evalStack.push(a - b);
        } else if (line == "MUL") {
            int b = evalStack.top(); evalStack.pop();
            int a = evalStack.top(); evalStack.pop();
            evalStack.push(a * b);
        } else if (line == "DIV") {
            int b = evalStack.top(); evalStack.pop();
            int a = evalStack.top(); evalStack.pop();
            evalStack.push(a / b);
        } else if (line == "PRINT") {
            int top = evalStack.top(); evalStack.pop();
            outFile << top << std::endl;
        }
    }

    outFile.close();
}
