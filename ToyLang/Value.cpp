#include "Value.h"
#include "Callable.hpp"

Value::Value()
	:tag(TypeTag::ERR), data{ 0 }
{}

Value::Value(bool val)
	: tag(TypeTag::BOOL)
{
	data.valBool = val;
}

Value::Value(double val)
	: tag(TypeTag::NUMBER)
{
	data.valNumber = val;
}

Value::Value(std::string val)
	: tag(TypeTag::STRING)
{
	data.valString = new char[val.size() + 1];
	val.copy(data.valString, val.size(), 0);
	data.valString[val.size()] = 0;
}

Value::Value(Callable* val)
	:tag(TypeTag::CALLABLE)
{
	data.valCallable = val;
}

Value::~Value()
{
	// Why??
}

void Value::print() const
{
	switch (tag)
	{
	case TypeTag::BOOL:
		std::cout << (data.valBool ? "true" : "false");
		break;
	case TypeTag::NUMBER:
		std::cout << data.valNumber;
		break;
	case TypeTag::STRING:
		std::cout << '"' << data.valString << '"';
		break;
	}
}

std::ostream& operator<<(std::ostream& os, const Value& val)
{
	switch (val.tag)
	{
	case TypeTag::BOOL:
		os << (val.data.valBool ? "true" : "false");
		break;
	case TypeTag::NUMBER:
		os << val.data.valNumber;
		break;
	case TypeTag::STRING:
		os << val.data.valString;
		break;
	default:
		break;
	}

	return os;
}
