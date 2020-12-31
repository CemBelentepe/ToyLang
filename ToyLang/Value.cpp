#include "Value.h"
#include "Callable.hpp"

Value::Value()
	:tag(TypeTag::ERR), data(nullptr)
{}

Value::Value(bool val)
	: tag(TypeTag::BOOL), data(val)
{
}

Value::Value(double val)
	: tag(TypeTag::NUMBER), data(val)
{
}

Value::Value(std::string val)
	: tag(TypeTag::STRING)
{
	data = new char[val.size() + 1];
	val.copy(std::get<char*>(data), val.size(), 0);
	std::get<char*>(data)[val.size()] = 0;
}

Value::Value(std::shared_ptr<Callable> val)
	:tag(TypeTag::CALLABLE), data(val)
{
}

void Value::print() const
{
	switch (tag)
	{
	case TypeTag::BOOL:
		std::cout << (std::get<bool>(data) ? "true" : "false");
		break;
	case TypeTag::NUMBER:
		std::cout << std::get<double>(data);
		break;
	case TypeTag::STRING:
		std::cout << '"' << std::get<char*>(data) << '"';
		break;
	default:
		break;
	}
}

std::ostream& operator<<(std::ostream& os, const Value& val)
{
	switch (val.tag)
	{
	case TypeTag::BOOL:
		std::cout << (std::get<bool>(val.data) ? "true" : "false");
		break;
	case TypeTag::NUMBER:
		std::cout << std::get<double>(val.data);
		break;
	case TypeTag::STRING:
		os << std::get<char*>(val.data);
		break;
	default:
		break;
	}

	return os;
}
