#pragma once
#include "AstVisitor.hpp"

class Enviroment;

class Interpreter : public ExprVisitor, public StmtVisitor
{
private:
	Value runtimeTypeError(Token errToken);
	std::vector<Stmt*> root;
public:
	Enviroment* enviroment;
	Enviroment* globals;

	Interpreter(std::vector<Stmt*> root);
	void run();

	Value visit(ExprBinary* expr);
	Value visit(ExprUnary* expr);
	Value visit(ExprLiteral* expr);
	Value visit(ExprVariableGet* expr);
	Value visit(ExprVariableSet* expr);
	Value visit(ExprCall* expr);

	void visit(StmtExpr* stmt);
	void visit(StmtFunction* stmt);
	void visit(StmtVarDecl* stmt);
};

