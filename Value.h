#pragma once
#include <string>
#include <iostream>
#include <variant>
#include <memory>

class Callable;
class ToyClass;
class ToyInstance;
enum class TypeTag
{
	ERR,
	BOOL,
	NUMBER,
	STRING,
	CALLABLE,
	INSTANCE
};

class Value
{
public:
	TypeTag tag;

	std::variant<void*, bool, double, std::string, std::shared_ptr<Callable>, std::shared_ptr<ToyInstance>> data;

	Value();
	Value(bool val);
	Value(double val);
	Value(std::string val);
	Value(std::shared_ptr<Callable> val);
	Value(std::shared_ptr<ToyInstance> val);

	void print() const;
	std::string toString();
	friend std::ostream& operator<<(std::ostream& os, const Value& val);
};
