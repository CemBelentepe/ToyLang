#pragma once

#include "Scanner.h"
#include "Value.h"

#include <memory>

class ExprVisitor;
class StmtVisitor;

enum class ExprType
{
	Binary, Unary, Literal, VariableGet, VariableSet, Call
};

class Expr
{
public:
	ExprType instance;
	Expr(ExprType instance)
		: instance(instance) {}
	virtual Value accept(ExprVisitor* visitor) = 0;
};

class ExprBinary : public Expr
{
public:
	std::unique_ptr<Expr> lhs;
	std::unique_ptr<Expr> rhs;
	Token op;

	ExprBinary(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, Token op)
		: Expr(ExprType::Binary), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
	{}

	Value accept(ExprVisitor* visitor) override;
};

class ExprUnary : public Expr
{
public:
	std::unique_ptr<Expr> rhs;
	Token op;

	ExprUnary(std::unique_ptr<Expr> rhs, Token op)
		: Expr(ExprType::Unary), rhs(std::move(rhs)), op(op)
	{}

	Value accept(ExprVisitor* visitor) override;
};

class ExprLiteral : public Expr
{
public:
	Value value;

	ExprLiteral(Value  value)
		:Expr(ExprType::Literal), value(value)
	{}

	Value accept(ExprVisitor* visitor) override;
};

class ExprVariableGet : public Expr
{
public:
	Token name;

	ExprVariableGet(Token name)
		: Expr(ExprType::VariableGet), name(name)
	{}

	Value accept(ExprVisitor* visitor) override;
};

class ExprVariableSet : public Expr
{
public:
	Token name;
	std::unique_ptr<Expr> setVal;
	Token op;

	ExprVariableSet(Token name, std::unique_ptr<Expr> setVal, Token op)
		:Expr(ExprType::VariableSet), name(name), setVal(std::move(setVal)), op(op)
	{}

	Value accept(ExprVisitor* visitor) override;
};

class ExprCall : public Expr
{
public:
	std::unique_ptr<Expr> callee;
	std::vector<std::unique_ptr<Expr>> args;
	Token paren;

	ExprCall(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args, Token paren)
		:Expr(ExprType::Call), callee(std::move(callee)), args(std::move(args)), paren(paren)
	{}

	Value accept(ExprVisitor* visitor) override;
};

class Stmt
{
public:
	virtual void accept(StmtVisitor* visitor) = 0;
};

class StmtExpr : public Stmt
{
public:
	std::unique_ptr<Expr> expr;

	StmtExpr(std::unique_ptr<Expr>&& expr)
		: expr(std::move(expr)) {}

	void accept(StmtVisitor* visitor) override;
};

class StmtFunction : public Stmt
{
public:
	Token name;
	std::vector<std::unique_ptr<Stmt>> stmts;
	std::vector<Token> params;

	StmtFunction(Token name, std::vector<std::unique_ptr<Stmt>> stmts, std::vector<Token> params)
		: name(name), stmts(std::move(stmts)), params(params) {}

	void accept(StmtVisitor* visitor) override;
};

class StmtVarDecl : public Stmt
{
public:
	Token name;
	std::unique_ptr<Expr> initVal;

	StmtVarDecl(Token name, std::unique_ptr<Expr> initVal = nullptr)
		: name(name), initVal(std::move(initVal))
	{}

	void accept(StmtVisitor* visitor) override;
};

class StmtBlock : public Stmt
{
public:
	std::vector<std::unique_ptr<Stmt>> stmts;

	StmtBlock(std::vector<std::unique_ptr<Stmt>> stmts)
		: stmts(std::move(stmts)){}

	void accept(StmtVisitor* visitor) override;
};

class StmtIf : public Stmt
{
public:
	std::unique_ptr<Expr> cond;
	Token paren;
	std::unique_ptr<Stmt> then;
	std::unique_ptr<Stmt> els;

	StmtIf(std::unique_ptr<Expr> cond, Token paren, std::unique_ptr<Stmt> then,	std::unique_ptr<Stmt> els)
		: cond(std::move(cond)), paren(paren), then(std::move(then)), els(std::move(els))
	{}

	void accept(StmtVisitor* visitor) override;
};
