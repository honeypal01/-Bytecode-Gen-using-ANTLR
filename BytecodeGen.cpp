#include "BytecodeGen.h"
#include "ExampleParser.h"
#include <iostream>
#include "SymbolTable.h"

void BytecodeGen::enterReAssignStat(ExampleParser::ReAssignStatContext *ctx) {
    std::string variable = ctx->ID()->getText();
    exitExpr(ctx->expr());
    bytecode.push_back("STORE " + variable);
    std::cout << "Re-Assign: " << variable << std::endl;
}

void BytecodeGen::enterDeclAssignStat(ExampleParser::DeclAssignStatContext *ctx) {
    std::string variable = ctx->ID()->getText();
    exitExpr(ctx->expr());
    bytecode.push_back("STORE " + variable);
    std::cout << "Declare & Assign: " << variable << std::endl;
}

void BytecodeGen::exitExpr(ExampleParser::ExprContext *ctx) {
    if (auto intLit = dynamic_cast<ExampleParser::IntegerLiteralContext*>(ctx)) {
        bytecode.push_back("PUSH " + intLit->INT()->getText());
    }
    else if (auto var = dynamic_cast<ExampleParser::VariableContext*>(ctx)) {
        bytecode.push_back("LOAD " + var->ID()->getText());
    }
    else if (auto addSub = dynamic_cast<ExampleParser::AddSubContext*>(ctx)) {
        // This runs after left and right have been evaluated
        std::string op = addSub->op->getText();
        if (op == "+") bytecode.push_back("ADD");
        else bytecode.push_back("SUB");
    }
    else if (auto mulDiv = dynamic_cast<ExampleParser::MulDivContext*>(ctx)) {
        std::string op = mulDiv->op->getText();
        if (op == "*") bytecode.push_back("MUL");
        else bytecode.push_back("DIV");
    }
}

void BytecodeGen::enterIf_stmt(ExampleParser::If_stmtContext *ctx) {
    std::string condition = ctx->expr()->getText();
    bytecode.push_back("// If condition: " + condition);
    bytecode.push_back("IF " + condition);
}

void BytecodeGen::enterWhile_stmt(ExampleParser::While_stmtContext *ctx) {
    std::string condition = ctx->expr()->getText();
    bytecode.push_back("// While loop condition: " + condition);
    bytecode.push_back("WHILE " + condition);
}

void BytecodeGen::exitAssignOpStat(ExampleParser::AssignOpStatContext *ctx) {
    std::string varName = ctx->ID()->getText();
    std::string op = ctx->op->getText();
    bytecode.push_back("LOAD " + varName);
    exitExpr(ctx->expr());
    bytecode.push_back("SWAP");
    if (op == "+") bytecode.push_back("ADD");
    else if (op == "-") bytecode.push_back("SUB");
    else if (op == "*") bytecode.push_back("MUL");
    else if (op == "/") bytecode.push_back("DIV");
    bytecode.push_back("STORE " + varName);
}

void traverseAST(std::shared_ptr<ASTNode> node, std::vector<std::string>& bytecode) {
    if (!node) return;
    if (node->type == "assign") {
        bytecode.push_back("PUSH " + node->right->value);
        bytecode.push_back("STORE " + node->left->value);
    } else if (node->type == "print") {
        bytecode.push_back("LOAD " + node->left->value);
        bytecode.push_back("PRINT");
    }
    for (auto& child : node->children) {
        traverseAST(child, bytecode);
    }
}

void BytecodeGen::generateBytecode(std::shared_ptr<ASTNode> astRoot) {
    traverseAST(astRoot, bytecode);
} 


void BytecodeGen::writeFinalOutputToFile(const std::string& path) {
    std::ofstream fout(path);
    for (const std::string& instr : bytecode) {
        if (instr.rfind("PRINT", 0) == 0) {
            // Find the previous LOAD instruction
            auto it = std::find_if(bytecode.rbegin(), bytecode.rend(), [&](const std::string& s) {
                return s.rfind("LOAD", 0) == 0;
            });

            if (it != bytecode.rend()) {
                std::string value = it->substr(5); // remove "LOAD "
                fout << value << std::endl;
            }
        }
    }
    fout.close();
}

void BytecodeGen::enterPrintStmt(ExampleParser::PrintStmtContext *ctx) {
    bytecode.push_back("// Printing value");

    // Evaluate expression (for bytecode)
    exitExpr(ctx->expr());
    bytecode.push_back("PRINT");

    // ✅ Evaluate result for finalOutput
    std::string result = (evaluateExpr(ctx->expr()));
    finalOutput.push_back(result);  // push evaluated result for final_output.txt
}

std::string BytecodeGen::evaluateExpr(ExampleParser::ExprContext* ctx) {
    // Very basic evaluation for now
    if (auto intLit = dynamic_cast<ExampleParser::IntegerLiteralContext*>(ctx)) {
        return intLit->INT()->getText();
    } else if (auto var = dynamic_cast<ExampleParser::VariableContext*>(ctx)) {
        std::string varName = var->ID()->getText();
        if (globalSymbolTable.isDeclared(varName)) {
            return globalSymbolTable.getSymbol(varName).value;
        } else {
            return "[undeclared variable: " + varName + "]"; 
        }
    }
    return "[expression evaluated]";
}

