#pragma once

#include "Callable.hpp"
#include "Value.h"

class ToyClass;

class ToyInstance
{
public:
	ToyClass* klass;
	std::unordered_map<std::string, Value> fields;
	ToyInstance(ToyClass* klass);

	Value get(Value instance, std::string name);
	void set(std::string name, Value val);
	Value bindTo(Value instance, Callable* callable);
};

class ToyClass : public Callable
{
public:
	StmtClass* klass;
	std::unordered_map<std::string, Value> methods;
	Callable* init;

	ToyClass(StmtClass* klass);
	Value getMethod(std::string name);

	Value call(Interpreter* interpreter, std::vector<Value> args) override;
	int arity() override;
	std::string name() override;
};
