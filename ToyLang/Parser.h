#pragma once

#include "Scanner.h"
#include "AST.h"

#include <iostream>
#include <memory>
#include <vector>

class Parser
{
private:
    std::vector<Token>& tokens;
    size_t currentToken;

public:
    Parser(std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

    std::unique_ptr<Stmt> decleration();
    std::unique_ptr<Stmt> function();
    std::unique_ptr<Stmt> varDecl();

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> block();
    std::unique_ptr<Stmt> ifStatement();

    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> multiplication();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();

    inline Token& advance()
    {
        return this->tokens[this->currentToken++];
    }
    inline Token& consumed() const
    {
        return this->tokens[this->currentToken - 1];
    }
    inline Token& peek() const
    {
        return this->tokens[this->currentToken];
    }
    inline Token& peekNext() const
    {
        return this->tokens[this->currentToken + 1];
    }
    bool match(TokenType type)
    {
        TokenType next = tokens[currentToken].type;
        if (next == type)
        {
            currentToken++;
            return true;
        }

        return false;
    }
    bool match(std::vector<TokenType> types)
    {
        for (auto& token : types)
        {
            if (match(token))
                return true;
        }
        return false;
    }
    void consume(TokenType type, const char* msg);

	void panic();

    inline std::unique_ptr<Expr> error(const char* message)
    {
        std::cout << message << std::endl;
        this->panic();
        return nullptr;
    }
    inline std::unique_ptr<Expr> errorAtToken(const char* message)
    {
        std::cout << "[line" << tokens[currentToken].line << "] Error" << message << std::endl;
        this->panic();
        return nullptr;
    }
};