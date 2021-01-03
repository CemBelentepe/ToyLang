#include "ToyClass.h"

ToyInstance::ToyInstance(ToyClass* klass)
	: klass(klass)
{ }

Value ToyInstance::get(Value instance, std::string name)
{
	auto it_fields = fields.find(name);
	if (it_fields != fields.end())
		return it_fields->second;

	Value function = klass->getMethod(name);
	if (function.tag != TypeTag::ERR)
	{
		return bindTo(instance, (std::get<std::shared_ptr<Callable>>(function.data).get()));
	}
	else
	{
		return Value();
	}
}

void ToyInstance::set(std::string name, Value val)
{
	auto it_fields = fields.find(name);
	if (it_fields != fields.end())
		it_fields->second = val;
	else
		fields[name] = val;
}

Value ToyInstance::bindTo(Value instance, Callable* callable)
{
	ToyFunction* func = (ToyFunction*)callable;
	std::shared_ptr<ToyFunction> method = std::make_shared<ToyFunction>(func->func);
	method->bind(instance);
	return Value(method);
}

ToyClass::ToyClass(StmtClass* klass)
	: klass(klass), init(nullptr)
{
	for (auto& m : klass->methods)
	{
		std::shared_ptr<Callable> method = std::make_shared<ToyFunction>(m.get());
		methods[m->name.getLexeme()] = Value(method);
	}

	auto it = methods.find("__init__");
	if (it != methods.end())
	{
		init = (std::get<std::shared_ptr<Callable>>(it->second.data)).get();
	}
}

Value ToyClass::getMethod(std::string name)
{
	auto it = methods.find(name);
	if (it != methods.end())
	{
		return it->second;
	}
	return Value();
}

Value ToyClass::call(Interpreter* interpreter, std::vector<Value> args)
{
	std::shared_ptr instance = std::make_shared<ToyInstance>(this);
	if (init)
	{
		Value init_mem = instance->bindTo(instance, init);
		std::get<std::shared_ptr<Callable>>(init_mem.data)->call(interpreter, args);
	}
	return Value(instance);
}

int ToyClass::arity()
{
	if (init)
		return init->arity();
	else
		return 0;
}

std::string ToyClass::name()
{
	return klass->name.getLexeme();
}
