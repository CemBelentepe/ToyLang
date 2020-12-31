#pragma once
#include <string>
#include <iostream>

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
	union Data
	{
		bool valBool;
		double valNumber;
		char* valString;
		Callable* valCallable;
	} data;

	Value();
	Value(bool val);
	Value(double val);
	Value(std::string val);
	Value(Callable* val);
	~Value();

	void print() const;

	friend std::ostream& operator<<(std::ostream& os, const Value& val);
};
