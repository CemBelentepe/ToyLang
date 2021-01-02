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

	Value* get(std::string name);
	void set(std::string name, Value val);
};

class ToyClass : public Callable
{
public:
	StmtClass* klass;
	std::unordered_map<std::string, Value> methods;

	ToyClass(StmtClass* klass);
	Value* getMethod(std::string name);

	Value call(Interpreter* interpreter, std::vector<Value> args) override;
	int arity() override;
	std::string name() override;
};
