#pragma once
#include "Callable.hpp"

class NativeArray;

class NativeArray : public ToyClass
{
public:
	class ArrayInstance : public ToyInstance
	{
	public:
		std::vector<Value> vec;
		ArrayInstance(NativeArray* klass)
			: ToyInstance(klass) {}
	};

	class MethodGet : public ToyFunction
	{
	public:
		MethodGet()
			: ToyFunction(nullptr)
		{}

		Value call(Interpreter* interpreter, std::vector<Value> args) override
		{
			size_t index = std::get<double>(args[0].data);
			return ((ArrayInstance*)std::get<std::shared_ptr<ToyInstance>>(self.data).get())->vec[index];
		}

		int arity() override
		{
			return 1;
		}

		std::string name() override
		{
			return "get";
		}

		Value bind(Value self) override
		{
			std::shared_ptr method = std::make_shared<MethodGet>();
			method->self = self;
			return Value(method);
		}
	};

	class MethodSet : public ToyFunction
	{
	public:

		MethodSet()
			: ToyFunction(nullptr)
		{}

		Value call(Interpreter* interpreter, std::vector<Value> args) override
		{
			size_t index = std::get<double>(args[0].data);
			((ArrayInstance*)std::get<std::shared_ptr<ToyInstance>>(self.data).get())->vec[index] = args[1];
			return args[1];
		}

		int arity() override
		{
			return 2;
		}

		std::string name() override
		{
			return "set";
		}

		Value bind(Value self) override
		{
			std::shared_ptr method = std::make_shared<MethodSet>();
			method->self = self;
			return Value(method);
		}
	};

	class MethodPush : public ToyFunction
	{
	public:
		MethodPush()
			: ToyFunction(nullptr)
		{}

		Value call(Interpreter* interpreter, std::vector<Value> args) override
		{
			((ArrayInstance*)std::get<std::shared_ptr<ToyInstance>>(self.data).get())->vec.push_back(args[0]);
			return Value();
		}

		int arity() override
		{
			return 1;
		}

		std::string name() override
		{
			return "push";
		}

		Value bind(Value self) override
		{
			std::shared_ptr method = std::make_shared<MethodPush>();
			method->self = self;
			return Value(method);
		}
	};

	class MethodPop : public ToyFunction
	{
	public:
		MethodPop()
			: ToyFunction(nullptr)
		{}

		Value call(Interpreter* interpreter, std::vector<Value> args) override
		{
			ArrayInstance* arr = (ArrayInstance*)std::get<std::shared_ptr<ToyInstance>>(self.data).get();
			Value back = arr->vec.back();
			arr->vec.pop_back();
			return back;
		}

		int arity() override
		{
			return 0;
		}

		std::string name() override
		{
			return "pop";
		}

		Value bind(Value self) override
		{
			std::shared_ptr method = std::make_shared<MethodPop>();
			method->self = self;
			return Value(method);
		}
	};

	class MethodSize : public ToyFunction
	{
	public:
		MethodSize()
			: ToyFunction(nullptr)
		{}

		Value call(Interpreter* interpreter, std::vector<Value> args) override
		{
			ArrayInstance* arr = (ArrayInstance*)std::get<std::shared_ptr<ToyInstance>>(self.data).get();
			return Value((double)arr->vec.size());
		}

		int arity() override
		{
			return 0;
		}

		std::string name() override
		{
			return "size";
		}

		Value bind(Value self) override
		{
			std::shared_ptr method = std::make_shared<MethodSize>();
			method->self = self;
			return Value(method);
		}
	};
public:
	NativeArray()
		: ToyClass("Array", {})
	{
		this->methods["get"] = Value(std::make_shared<MethodGet>());
		this->methods["set"] = Value(std::make_shared<MethodSet>());
		this->methods["__iget__"] = Value(std::make_shared<MethodGet>());
		this->methods["__iset__"] = Value(std::make_shared<MethodSet>());
		this->methods["push"] = Value(std::make_shared<MethodPush>());
		this->methods["pop"] = Value(std::make_shared<MethodPop>());
		this->methods["size"] = Value(std::make_shared<MethodSize>());
	}

	Value call(Interpreter* interpreter, std::vector<Value> args) override
	{
		std::shared_ptr<ToyInstance> instance = std::make_shared<ArrayInstance>(this);
		return Value(instance);
	}
};
