#pragma once
#include "AstVisitor.hpp"
#include <sstream>

class Enviroment;

class Interpreter : public ExprVisitor, public StmtVisitor
{
private:
	Value runtimeTypeError(Token errToken);
	std::vector<Stmt*> root;
	std::stringstream err;
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
	Value visit(ExprMemberGet* expr);
	Value visit(ExprMemberSet* expr);
	Value visit(ExprArrayGet* expr);
	Value visit(ExprArraySet* expr);

	void visit(StmtExpr* stmt);
	void visit(StmtFunction* stmt);
	void visit(StmtVarDecl* stmt);
	void visit(StmtBlock* stmt);
	void visit(StmtIf* stmt);
	void visit(StmtWhile* stmt);
	void visit(StmtReturn* stmt);
	void visit(StmtClass* stmt);
};

