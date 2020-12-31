#pragma once
#include "AST.h"

class ExprVisitor
{
public:
    virtual Value visit(ExprBinary* expr) = 0;
    virtual Value visit(ExprUnary* expr) = 0;
    virtual Value visit(ExprLiteral* expr) = 0;
    virtual Value visit(ExprVariableGet* expr) = 0;
    virtual Value visit(ExprVariableSet* expr) = 0;
    virtual Value visit(ExprCall* expr) = 0;
};

class StmtVisitor
{
public:
    virtual void visit(StmtExpr* stmt) = 0;
    virtual void visit(StmtFunction* stmt) = 0;
    virtual void visit(StmtVarDecl* stmt) = 0;
};
