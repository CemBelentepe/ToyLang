#pragma once
#include "Value.h"
#include "AST.h"
#include "Interpreter.h"
#include "Enviroment.hpp"

class Callable
{
public:
	virtual ~Callable() = default;
	virtual Value call(Interpreter* interpreter, std::vector<Value> args) = 0;
	virtual int arity() = 0;
};

class ToyFunction : public Callable
{
public:
	StmtFunction* func;

	ToyFunction(StmtFunction* func)
		: func(func) {}

	Value call(Interpreter* interpreter, std::vector<Value> args) override
	{
		Enviroment* env = interpreter->enviroment;
		interpreter->enviroment = new Enviroment(interpreter->globals);
		for (size_t i = 0; i < args.size(); i++)
		{
			interpreter->enviroment->define(func->params[i], args[i]);
		}

		for (auto& s : func->stmts)
			s->accept(interpreter);
		
		delete interpreter->enviroment;
		interpreter->enviroment = env;

		return Value(); // add return here!
	}

	int arity() override
	{
		return func->params.size();
	}
};

class NativePrint : public Callable
{
public:
	Value call(Interpreter* interpreter, std::vector<Value> args) override
	{
		std::cout << args[0];
		return Value();
	}

	int arity()
	{
		return 1;
	}
};
