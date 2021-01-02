#include "ToyClass.h"

ToyInstance::ToyInstance(ToyClass* klass)
	: klass(klass) 
{ }

Value* ToyInstance::get(std::string name)
{
	auto it_fields = fields.find(name);
	if (it_fields != fields.end())
		return &it_fields->second;
	return klass->getMethod(name);
}

void ToyInstance::set(std::string name, Value val)
{
	Value* value = get(name);
	if (value)
		*value = val;
	else
		fields[name] = val;
}

ToyClass::ToyClass(StmtClass* klass)
	: klass(klass)
{
	for (auto& m : klass->methods)
	{
		std::shared_ptr<Callable> method = std::make_shared<ToyFunction>(m.get());
		methods[m->name.getLexeme()] = Value(method);
	}
}

Value* ToyClass::getMethod(std::string name)
{
	auto it = methods.find(name);
	if (it != methods.end())
		return &it->second;
	return nullptr;
}

Value ToyClass::call(Interpreter * interpreter, std::vector<Value> args)
{
	// TODO add constructor
	Value val(std::make_shared<ToyInstance>(this));
	return val;
}

int ToyClass::arity()
{
	return 0;
}

std::string ToyClass::name()
{
	return klass->name.getLexeme();
}
