#include "Value.h"
#include "Callable.hpp"
#include "ToyClass.h"

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

Value::Value(std::shared_ptr<ToyInstance> val)
	: tag(TypeTag::INSTANCE), data(val)
{
}

void Value::print() const
{
	std::cout << *this;
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
	case TypeTag::CALLABLE:
		os << std::get<std::shared_ptr<Callable>>(val.data)->name();
		break;
	case TypeTag::INSTANCE:
		os << std::get<std::shared_ptr<ToyInstance>>(val.data)->klass->name() << " instance";
		break;
	default:
		break;
	}

	return os;
}
