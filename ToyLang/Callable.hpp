#pragma once
#include "Value.h"
#include "AST.h"
#include "Interpreter.h"
#include "Enviroment.hpp"

#include <ctime>

class Callable
{
public:
	virtual ~Callable() = default;
	virtual Value call(Interpreter* interpreter, std::vector<Value> args) = 0;
	virtual int arity() = 0;
	virtual std::string name() = 0;
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

		Value ret;
		try
		{
			for (auto& s : func->stmts)
				s->accept(interpreter);
		}
		catch (Value val)
		{
			ret = val;
		}

		delete interpreter->enviroment;
		interpreter->enviroment = env;

		return ret; // add return here!
	}

	int arity() override
	{
		return func->params.size();
	}

	std::string name() override
	{
		return func->name.getLexeme();
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

	std::string name() override
	{
		return "print";
	}
};

class NativeClock : public Callable
{
public:
	Value call(Interpreter* interpreter, std::vector<Value> args) override
	{
		return (double)clock() / CLOCKS_PER_SEC;
	}

	int arity()
	{
		return 0;
	}

	std::string name() override
	{
		return "clock";
	}
};