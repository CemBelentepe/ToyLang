#pragma once
#include <string>
#include <iostream>
#include <variant>

class Callable;
enum class TypeTag
{
	ERR,
	BOOL,
	NUMBER,
	STRING,
	CALLABLE
};

class Value
{
public:
	TypeTag tag;

	std::variant<bool, double, char*, std::shared_ptr<Callable>> data;

	Value();
	Value(bool val);
	Value(double val);
	Value(std::string val);
	Value(std::shared_ptr<Callable> val);

	void print() const;

	friend std::ostream& operator<<(std::ostream& os, const Value& val);
};
