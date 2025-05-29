#include "SymbolTable.h"
#include <iostream>
#include <fstream>
#include "SymbolTable.h"


SymbolTable globalSymbolTable;  // 🔥 Declare here

// Declare a variable with its name, type, and value
void SymbolTable::declareVariable(const std::string &varName, const std::string &type, const std::string &value) {
    Symbol symbol = {type, value};
    symbolMap[varName] = symbol;  // Store the symbol with type and value
}

// Check if a variable is already declared
bool SymbolTable::isDeclared(const std::string &varName) const {
    return symbolMap.find(varName) != symbolMap.end();
}

// Update the value of an existing variable
void SymbolTable::updateValue(const std::string &varName, const std::string &value) {
    if (isDeclared(varName)) {
        symbolMap[varName].value = value;  // Update only the value
    }
}

// Declare a function with its name, type, and parameter count
void SymbolTable::declareFunction(const std::string &funcName, const std::string &type, const std::string &paramCount) {
    Symbol funcSymbol = {type, paramCount}; // Functions are stored with type and parameter count
    functionMap[funcName] = funcSymbol;  // Store the function symbol
}

// Check if a function is already declared
bool SymbolTable::isFunctionDeclared(const std::string &funcName) const {
    return functionMap.find(funcName) != functionMap.end();
}

// Get a reference to the symbol map (variables and functions)
const std::unordered_map<std::string, Symbol>& SymbolTable::getSymbolMap() const {
    return symbolMap;
}

Symbol SymbolTable::getSymbol(const std::string &varName) const {
    auto it = symbolMap.find(varName);
    if (it != symbolMap.end()) {
        return it->second;
    }
    return {"unknown", "??"};  // Return dummy symbol if not found
}


// Write both variables and functions to a file
void SymbolTable::writeToFile(const std::string &filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return;

    // Write variables
    out << "Declared Variables (with types and values):\n";
    for (const auto &entry : symbolMap) {
        out << "- " << entry.first << " (" << entry.second.type << ") = " << entry.second.value << "\n";
    }

    // Write functions
    out << "\nDeclared Functions (with types and parameter count):\n";
    for (const auto &entry : functionMap) {
        out << "- " << entry.first << " (" << entry.second.type << ") with " << entry.second.value << " parameters\n";
    }

    out.close();
}
