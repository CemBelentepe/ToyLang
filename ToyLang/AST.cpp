#include "AST.h"
#include "AstVisitor.hpp"

Value ExprBinary::accept(ExprVisitor* visitor)
{
    return visitor->visit(this);
}

Value ExprUnary::accept(ExprVisitor* visitor)
{
    return visitor->visit(this);
}

Value ExprLiteral::accept(ExprVisitor* visitor)
{
    return visitor->visit(this);
}

Value ExprVariableGet::accept(ExprVisitor* visitor)
{
    return visitor->visit(this);
}

Value ExprVariableSet::accept(ExprVisitor* visitor)
{
    return visitor->visit(this);
}

Value ExprCall::accept(ExprVisitor* visitor)
{
    return visitor->visit(this);
}

void StmtExpr::accept(StmtVisitor* visitor)
{
    return visitor->visit(this);
}

void StmtFunction::accept(StmtVisitor* visitor)
{
    return visitor->visit(this);
}

void StmtVarDecl::accept(StmtVisitor* visitor)
{
    return visitor->visit(this);
}