#include "SemanticChecker.h"
#include <fstream>
#include <iostream>
#include <regex>

const std::vector<std::string>& SemanticChecker::getSemanticErrors() const {
    return semanticErrors;
}

// ✅ Updated to handle arithmetic expressions: +, -, *, /
std::pair<std::string, std::string> SemanticChecker::inferConstantType(const std::string &expr) {
    std::string trimmed = std::regex_replace(expr, std::regex("\\s+"), ""); // Remove spaces

    // Handle parentheses
    if (trimmed.front() == '(' && trimmed.back() == ')') {
        return inferConstantType(trimmed.substr(1, trimmed.size() - 2));
    }

    // Check if it's a known variable
    if (symbols.isDeclared(trimmed)) {
        Symbol entry = symbols.getSymbol(trimmed);
        if (entry.type == "int") {
            return {entry.type, entry.value};
        } else {
            return {"unknown", "??"};
        }
    }

    // Try direct int
    try {
        int val = std::stoi(trimmed);
        return {"int", std::to_string(val)};
    } catch (...) {}

    // Check for operators: *, /, +, -
    std::vector<std::string> ops = {"*", "/", "+", "-"};
    for (const auto& op : ops) {
        size_t pos = trimmed.find_last_of(op);
        if (pos != std::string::npos) {
            std::string left = trimmed.substr(0, pos);
            std::string right = trimmed.substr(pos + 1);

            auto leftResult = inferConstantType(left);
            auto rightResult = inferConstantType(right);

            if (leftResult.first == "int" && rightResult.first == "int") {
                int l = std::stoi(leftResult.second);
                int r = std::stoi(rightResult.second);
                int res = 0;
                if (op == "+") res = l + r;
                else if (op == "-") res = l - r;
                else if (op == "*") res = l * r;
                else if (op == "/") res = r != 0 ? l / r : 0;
                return {"int", std::to_string(res)};
            }
        }
    }

    return {"unknown", "??"};
}



void SemanticChecker::enterDeclAssignStat(ExampleParser::DeclAssignStatContext *ctx) {
    std::string varName = ctx->ID()->getText();
    std::string valueText = ctx->expr()->getText();

    if (symbols.isDeclared(varName)) {
        semanticErrors.push_back("❌ Semantic error: Variable '" + varName + "' is already declared.");
    } else {
        auto [type, value] = inferConstantType(valueText);
        if (type == "unknown") {
            semanticErrors.push_back("❌ Semantic error: Cannot infer type of '" + valueText + "' for variable '" + varName + "'.");
            symbols.declareVariable(varName, "unknown", "??");
        } else {
            symbols.declareVariable(varName, type, value);
        }
        std::cout << "🟢 Declared: " << varName << " = " << valueText << " (" << type << ")" << std::endl;
    }
}

void SemanticChecker::enterReAssignStat(ExampleParser::ReAssignStatContext *ctx) {
    std::string varName = ctx->ID()->getText();
    std::string valueText = ctx->expr()->getText();

    if (!symbols.isDeclared(varName)) {
        semanticErrors.push_back("❌ Semantic error: Variable '" + varName + "' used before declaration.");
    } else {
        auto [type, value] = inferConstantType(valueText);
        if (type != "unknown") {
            symbols.updateValue(varName, value);
        }
        std::cout << "🔵 Reassigned: " << varName << " = " << valueText << std::endl;
    }
}

void SemanticChecker::writeErrorsToFile(const std::string &filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing errors.\n";
        return;
    }

    for (const auto &err : semanticErrors) {
        out << err << "\n";
    }

    out.close();
}

const SymbolTable& SemanticChecker::getSymbolTable() const {
    return symbols;
}

void SemanticChecker::writeSymbolTableToFile(const std::string &filename) const {
    symbols.writeToFile(filename);
}

void SemanticChecker::writeAllToFile(const std::string &filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing all info.\n";
        return;
    }

    for (const auto &err : semanticErrors) {
        out << err << "\n";
    }

    out << "\nDeclared Variables:\n";
    for (const auto &entry : symbols.getSymbolMap()) {
        out << "- " << entry.first << " | " << entry.second.type << " | " << entry.second.value << "\n";
    }

    out.close();
}

bool SemanticChecker::hasErrors() const {
    return !semanticErrors.empty();
}
