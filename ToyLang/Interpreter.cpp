#include "Interpreter.h"
#include <sstream>
#include <iostream>

#include "Enviroment.hpp"
#include "Callable.hpp"

Value Interpreter::runtimeTypeError(Token errToken)
{
	std::cout << "[ERROR] Invalid type for operand '" << errToken.getLexeme() << "' at line: " << errToken.line << std::endl;
	return Value();
}

Interpreter::Interpreter(std::vector<Stmt*> root)
	:root(root), enviroment(nullptr), globals(nullptr)
{}

void Interpreter::run()
{
	globals = new Enviroment();
	enviroment = globals;

	enviroment->define("print", Value(std::make_shared<NativePrint>()));

	for (auto& stmt : root)
		stmt->accept(this);
	std::get<std::shared_ptr<Callable>>(enviroment->getVar("main").data)->call(this, {});

	delete globals;
}

Value Interpreter::visit(ExprBinary* expr)
{
	Value a = expr->lhs->accept(this);

	if (a.tag == TypeTag::BOOL)
	{
		switch (expr->op.type)
		{
		case TokenType::AND:
			if (std::get<bool>(a.data))
			{
				Value b = expr->rhs->accept(this);
				if (b.tag == TypeTag::BOOL)
					return b;
			}
			else
			{
				return false;
			}
			break;
		case TokenType::OR:
			if (std::get<bool>(a.data))
				return true;
			Value b = expr->rhs->accept(this);
			if (b.tag == TypeTag::BOOL)
				return b;
			break;
		}
	}
	else {
		Value b = expr->rhs->accept(this);

		if (a.tag == TypeTag::NUMBER && b.tag == TypeTag::NUMBER) {
			switch (expr->op.type)
			{
			case TokenType::PLUS:
				return std::get<double>(a.data) + std::get<double>(b.data);
			case TokenType::MINUS:
				return std::get<double>(a.data) - std::get<double>(b.data);
			case TokenType::STAR:
				return std::get<double>(a.data) * std::get<double>(b.data);
			case TokenType::SLASH:
				return std::get<double>(a.data) / std::get<double>(b.data);
			}
		}
		else if (a.tag == TypeTag::STRING && b.tag == TypeTag::STRING)
		{
			std::stringstream ss;
			ss << std::get<char*>(a.data) << std::get<char*>(b.data);
			return ss.str();
		}
	}

	return runtimeTypeError(expr->op);
}

Value Interpreter::visit(ExprUnary* expr)
{
	Value a = expr->rhs->accept(this);
	if (a.tag == TypeTag::NUMBER)
	{
		return -std::get<double>(a.data);
	}

	return runtimeTypeError(expr->op);
}

Value Interpreter::visit(ExprVariableGet* expr)
{
	return enviroment->getVar(expr->name.getLexeme());
}

Value Interpreter::visit(ExprVariableSet* expr)
{
	Value val = expr->setVal->accept(this);
	std::string name = expr->name.getLexeme();
	if (expr->op.type != TokenType::EQUAL)
	{
		Value orig = enviroment->getVar(name);
		if (val.tag == TypeTag::NUMBER && orig.tag == TypeTag::NUMBER)
		{
			switch (expr->op.type)
			{
			case TokenType::PLUS_EQUAL:
				val.data = std::get<double>(orig.data) + std::get<double>(val.data);
				break;
			case TokenType::MINUS_EQUAL:
				val.data = std::get<double>(orig.data) - std::get<double>(val.data);
				break;
			case TokenType::STAR_EQUAL:
				val.data = std::get<double>(orig.data) * std::get<double>(val.data);
				break;
			case TokenType::SLASH_EQUAL:
				val.data = std::get<double>(orig.data) / std::get<double>(val.data);
				break;
			}
		}
		else
		{
			return runtimeTypeError(expr->op);
		}
	}
	enviroment->setVar(name, val);
	return val;
}

Value Interpreter::visit(ExprCall* expr)
{
	Value func = expr->callee->accept(this);
	if (func.tag != TypeTag::CALLABLE)
	{
		std::cout << "[ERROR] Invalid function call at line: " << expr->paren.line << std::endl;
		return Value();
	}
	else if (std::get<std::shared_ptr<Callable>>(func.data)->arity() == expr->args.size())
	{
		std::vector<Value> args;
		for (auto& a : expr->args)
		{
			args.push_back(a->accept(this));
		}
		return std::get<std::shared_ptr<Callable>>(func.data)->call(this, args);
	}
	else
	{
		std::cout << "[ERROR] Invalid function call with invalid argument number at line: " << expr->paren.line << std::endl;
		return Value();
	}
}

Value Interpreter::visit(ExprLiteral* expr)
{
	return expr->value;
}

void Interpreter::visit(StmtExpr* stmt)
{
	Value a = stmt->expr->accept(this);
	// std::cout << a << std::endl;
}

void Interpreter::visit(StmtFunction* stmt)
{
	Value funcVal(std::make_shared<ToyFunction>(stmt));
	enviroment->define(stmt->name, funcVal);
}

void Interpreter::visit(StmtVarDecl* stmt)
{
	enviroment->define(stmt->name, stmt->initVal->accept(this));
}

void Interpreter::visit(StmtBlock* stmt)
{
	Enviroment* env = this->enviroment;
	this->enviroment = new Enviroment(env);

	for (auto& s : stmt->stmts)
		s->accept(this);

	delete this->enviroment;
	this->enviroment = env;
}

void Interpreter::visit(StmtIf* stmt)
{
	Value res = stmt->cond->accept(this);

	if (res.tag == TypeTag::BOOL) {
		if (std::get<bool>(res.data))
			stmt->then->accept(this);
		else if (stmt->els)
			stmt->els->accept(this);
	}
	else
	{
		std::cout << "Invalid data type for if statement at line: " << stmt->paren.line << std::endl;
	}
}
