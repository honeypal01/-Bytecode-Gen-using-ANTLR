#include "Optimizer.h"
#include "ASTNode.h"
#include <string>
#include <stdexcept>
#include <memory>

// Perform constant folding when exiting an expression node
void Optimizer::exitExpr(ExampleParser::ExprContext *ctx) {
    if (ctx->children.size() == 3) {
        std::string left = ctx->children[0]->getText();
        std::string op = ctx->children[1]->getText();
        std::string right = ctx->children[2]->getText();

        try {
            int leftVal = std::stoi(left);
            int rightVal = std::stoi(right);
            int result = 0;

            if (op == "+") result = leftVal + rightVal;
            else if (op == "-") result = leftVal - rightVal;
            else if (op == "*") result = leftVal * rightVal;
            else if (op == "/" && rightVal != 0) result = leftVal / rightVal;
            else return;

            foldedValues[ctx] = std::to_string(result);
        } catch (...) {
            // Non-integer expression; skip
        }
    }
}

// Constant folding on AST
std::shared_ptr<ASTNode> Optimizer::optimize(std::shared_ptr<ASTNode> root) {
    if (!root) return nullptr;

    std::cout << "Optimizing node: " << (root->isExpr ? root->op : root->value) << std::endl;

    // Recur first
    root->left = optimize(root->left);
    root->right = optimize(root->right);

    // Now handle expression folding
    if (root->isExpr && root->left && root->right) {
        std::cout << "Checking for constant folding on: " << root->op << std::endl;

        if (root->left->isConstant() && root->right->isConstant()) {
            int leftVal = root->left->getConstantValue();
            int rightVal = root->right->getConstantValue();
            int result = 0;

            std::cout << "Left value: " << leftVal << ", Right value: " << rightVal << std::endl;

            if (root->op == "+") result = leftVal + rightVal;
            else if (root->op == "-") result = leftVal - rightVal;
            else if (root->op == "*") result = leftVal * rightVal;
            else if (root->op == "/" && rightVal != 0) result = leftVal / rightVal;
            else {
                std::cout << "Invalid operation: " << root->op << std::endl;
                return root; // skip invalid cases
            }

            // Folded into constant node
            std::cout << "Optimization result: " << result << std::endl;
            return std::make_shared<ASTNode>(std::to_string(result), false);  // not an expr now
        }
    }

    return root;
}





