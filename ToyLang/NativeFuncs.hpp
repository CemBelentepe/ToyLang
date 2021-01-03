#pragma once
#include "Callable.hpp"

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

class NativeStr : public Callable
{
public:
	Value call(Interpreter* interpreter, std::vector<Value> args) override
	{
		return Value(args[0].toString());
	}

	int arity()
	{
		return 1;
	}

	std::string name() override
	{
		return "str";
	}
};
