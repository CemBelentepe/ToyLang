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
	Value self;

	ToyFunction(StmtFunction* func)
		: func(func) {}

	virtual Value call(Interpreter* interpreter, std::vector<Value> args) override
	{
		Enviroment* env = interpreter->enviroment;
		interpreter->enviroment = new Enviroment(interpreter->globals);
		interpreter->enviroment->define("self", self);
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

		return ret;
	}

	virtual int arity() override
	{
		return func->params.size();
	}

	virtual std::string name() override
	{
		return func->name.getLexeme();
	}

	virtual Value bind(Value self)
	{
		std::shared_ptr<ToyFunction> method = std::make_shared<ToyFunction>(func);
		method->self = self;
		return Value(method);
	}
};
