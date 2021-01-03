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
	virtual Value bindTo(Value instance, Callable* callable);
};

class ToyClass : public Callable
{
public:
	std::string m_name;
	std::unordered_map<std::string, Value> methods;
	Callable* init;

	ToyClass(std::string m_name, const std::vector<std::unique_ptr<StmtFunction>>& stmt_methods);
	virtual Value getMethod(std::string name);

	virtual Value call(Interpreter* interpreter, std::vector<Value> args) override;
	virtual int arity() override;
	virtual std::string name() override;
};
