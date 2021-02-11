#include "Interpreter.h"
#include <iostream>
#include <string>
#include <sstream>
#include <variant>

#include "Enviroment.hpp"
#include "Callable.hpp"
#include "ToyClass.h"
#include "NativeArray.hpp"
#include "NativeFuncs.hpp"
#include "Value.h"

Value Interpreter::runtimeTypeError(Token errToken)
{
	err << "[ERROR] Invalid type for operand '" << errToken.getLexeme() << "' at line: " << errToken.line << std::endl;
	throw err.str();
	return Value();
}

Interpreter::Interpreter(std::vector<Stmt*> root)
	:root(root), enviroment(nullptr), globals(nullptr)
{}

void Interpreter::run()
{
	globals = new Enviroment();
	enviroment = globals;

	enviroment->define("print", Value(std::make_shared<NativePrint>()));
	enviroment->define("input", Value(std::make_shared<NativeInput>()));
	enviroment->define("clock", Value(std::make_shared<NativeClock>()));
	enviroment->define("str", Value(std::make_shared<NativeStr>()));
	enviroment->define("Array", Value(std::make_shared<NativeArray>()));

	try
	{
		for (auto& stmt : root)
			stmt->accept(this);
		std::get<std::shared_ptr<Callable>>(enviroment->getVar("main").data)->call(this, {});
	}
	catch (std::string err)
	{
		std::cout << err;
	}

	delete globals;
}

Value Interpreter::visit(ExprBinary* expr)
{
	Value a = expr->lhs->accept(this);

	if (a.tag == TypeTag::INSTANCE)
	{
		Value b = expr->rhs->accept(this);
		auto callMethod = [this](Value& a, Value& b, const std::string& name) {
			Value func = std::get<std::shared_ptr<ToyInstance>>(a.data)->get(a, name);
			Callable* callable = std::get<std::shared_ptr<Callable>>(func.data).get();
			if (func.tag == TypeTag::CALLABLE)
			{
				if (callable->arity() == 1)
					return callable->call(this, { b });
				else
				{
					err << "[ERROR] Invalid function call with invalid argument count\n";
					throw err.str();
					return Value();
				}
			}
			else
			{
				err << "[ERROR] Type " << std::get<std::shared_ptr<ToyInstance>>(a.data)->klass->name() << " does not have '" << name << "' method.\n";
				throw err.str();
				return Value();
			}
		};

		switch (expr->op.type)
		{
		case TokenType::PLUS:
			return callMethod(a, b, "__add__");
		case TokenType::MINUS:
			return callMethod(a, b, "__sub__");
		case TokenType::STAR:
			return callMethod(a, b, "__mul__");
		case TokenType::SLASH:
			return callMethod(a, b, "__div__");
		case TokenType::LESS:
			return callMethod(a, b, "__les__");
		case TokenType::GREAT:
			return callMethod(a, b, "__grt__");
		case TokenType::LESS_EQUAL:
			return callMethod(a, b, "__lte__");
		case TokenType::GREAT_EQUAL:
			return callMethod(a, b, "__gte__");
		case TokenType::EQUAL_EQUAL:
			return callMethod(a, b, "__equ__");
		case TokenType::BANG_EQUAL:
			return callMethod(a, b, "__neq__");
		}

	}
	else if (expr->op.type == TokenType::EQUAL_EQUAL)
	{
		Value b = expr->rhs->accept(this);
		return a.data == b.data;
	}
	else if (expr->op.type == TokenType::BANG_EQUAL)
	{
		Value b = expr->rhs->accept(this);
		return a.data != b.data;
	}
	else if (a.tag == TypeTag::BOOL)
	{
		switch (expr->op.type)
		{
		case TokenType::AND:
			if (std::get<bool>(a.data))
			{
				Value b = expr->rhs->accept(this);
				if (b.tag == TypeTag::BOOL)
					return b;
			}
			else
			{
				return false;
			}
			break;
		case TokenType::OR:
			if (std::get<bool>(a.data))
				return true;
			Value b = expr->rhs->accept(this);
			if (b.tag == TypeTag::BOOL)
				return b;
			break;
		}
	}
	else {
		Value b = expr->rhs->accept(this);

		if (a.tag == TypeTag::NUMBER && b.tag == TypeTag::NUMBER) {
			switch (expr->op.type)
			{
			case TokenType::PLUS:
				return std::get<double>(a.data) + std::get<double>(b.data);
			case TokenType::MINUS:
				return std::get<double>(a.data) - std::get<double>(b.data);
			case TokenType::STAR:
				return std::get<double>(a.data) * std::get<double>(b.data);
			case TokenType::SLASH:
				return std::get<double>(a.data) / std::get<double>(b.data);
			case TokenType::LESS:
				return std::get<double>(a.data) < std::get<double>(b.data);
			case TokenType::GREAT:
				return std::get<double>(a.data) > std::get<double>(b.data);
			case TokenType::LESS_EQUAL:
				return std::get<double>(a.data) <= std::get<double>(b.data);
			case TokenType::GREAT_EQUAL:
				return std::get<double>(a.data) >= std::get<double>(b.data);
			}
		}
		else if (a.tag == TypeTag::STRING && b.tag == TypeTag::STRING && expr->op.type == TokenType::PLUS)
		{
			std::stringstream ss;
			ss << std::get<std::string>(a.data) << std::get<std::string>(b.data);
			return ss.str();
		}
	}

	return runtimeTypeError(expr->op);
}

Value Interpreter::visit(ExprUnary* expr)
{
	auto callMethod = [this](Value& a, const std::string& name) {
		Value func = std::get<std::shared_ptr<ToyInstance>>(a.data)->get(a, name);
		Callable* callable = std::get<std::shared_ptr<Callable>>(func.data).get();
		if (func.tag == TypeTag::CALLABLE)
		{
			if (callable->arity() == 0)
				return callable->call(this, { });
			else
			{
				err << "[ERROR] Invalid function call with invalid argument count\n";
				throw err.str();
				return Value();
			}
		}
		else
		{
			err << "Type " << std::get<std::shared_ptr<ToyInstance>>(a.data)->klass->name() << " does not have '" << name << "' method.\n";
			throw err.str();
			return Value();
		}
	};

	Value a = expr->rhs->accept(this);
	switch (expr->op.type)
	{
	case TokenType::MINUS:
		if (a.tag == TypeTag::NUMBER)
			return -std::get<double>(a.data);
		else if (a.tag == TypeTag::INSTANCE)
			return callMethod(a, "__neg__");
		break;
	case TokenType::BANG:
		if (a.tag == TypeTag::BOOL)
			return !std::get<bool>(a.data);
		else if (a.tag == TypeTag::INSTANCE)
			return callMethod(a, "__not__");
		break;
	}

	return runtimeTypeError(expr->op);
}

Value Interpreter::visit(ExprVariableGet* expr)
{
	return enviroment->getVar(expr->name.getLexeme());
}

Value Interpreter::visit(ExprVariableSet* expr)
{
	Value val = expr->setVal->accept(this);
	std::string name = expr->name.getLexeme();
	if (expr->op.type != TokenType::EQUAL)
	{
		Value orig = enviroment->getVar(name);
		if (val.tag == TypeTag::NUMBER && orig.tag == TypeTag::NUMBER)
		{
			switch (expr->op.type)
			{
			case TokenType::PLUS_EQUAL:
				val.data = std::get<double>(orig.data) + std::get<double>(val.data);
				break;
			case TokenType::MINUS_EQUAL:
				val.data = std::get<double>(orig.data) - std::get<double>(val.data);
				break;
			case TokenType::STAR_EQUAL:
				val.data = std::get<double>(orig.data) * std::get<double>(val.data);
				break;
			case TokenType::SLASH_EQUAL:
				val.data = std::get<double>(orig.data) / std::get<double>(val.data);
				break;
			}
		}
		else if (val.tag == TypeTag::STRING && orig.tag == TypeTag::STRING)
		{
			std::stringstream ss;
			ss << std::get<std::string>(orig.data) << std::get<std::string>(val.data);
			val.data = ss.str();
		}
		else if (orig.tag == TypeTag::INSTANCE)
		{
			auto callMethod = [this](Value& a, Value& b, const std::string& name) {
				Value func = std::get<std::shared_ptr<ToyInstance>>(a.data)->get(a, name);
				Callable* callable = std::get<std::shared_ptr<Callable>>(func.data).get();
				if (func.tag == TypeTag::CALLABLE)
				{
					if (callable->arity() == 1)
						return callable->call(this, { b });
					else
					{
						err << "[ERROR] Invalid function call with invalid argument count\n";
						throw err.str();
						return Value();
					}
				}
				else
				{
					err << "Type " << std::get<std::shared_ptr<ToyInstance>>(a.data)->klass->name() << " does not have '" << name << "' method.\n";
					throw err.str();
					return Value();
				}
			};
			switch (expr->op.type)
			{
			case TokenType::PLUS_EQUAL:
				val = callMethod(orig, val, "__iadd__");
				break;
			case TokenType::MINUS_EQUAL:
				val = callMethod(orig, val, "__isub__");
				break;
			case TokenType::STAR_EQUAL:
				val = callMethod(orig, val, "__imul__");
				break;
			case TokenType::SLASH_EQUAL:
				val = callMethod(orig, val, "__idiv__");
				break;
			}
		}
		else
		{
			return runtimeTypeError(expr->op);
		}
	}
	enviroment->setVar(name, val);
	return val;
}

Value Interpreter::visit(ExprMemberGet* expr)
{
	Value object = expr->object->accept(this);
	if (object.tag == TypeTag::INSTANCE)
	{
		std::string name = expr->name.getLexeme();
		auto instance = std::get<std::shared_ptr<ToyInstance>>(object.data);
		Value mem = instance->get(instance, name);
		if (mem.tag != TypeTag::ERR)
			return mem;
		else
		{
			err << "[ERROR] Object does not contain the member " << name << " at line: " << expr->name.line << ".\n";
			throw err.str();
			return Value();
		}
	}
	else
	{
		err << "[ERROR] Getter can only work on classes line: " << expr->name.line << ".\n";
		throw err.str();
		return Value();
	}

}

Value Interpreter::visit(ExprMemberSet* expr)
{
	Value object = expr->object->accept(this);
	if (object.tag == TypeTag::INSTANCE)
	{
		std::string name = expr->name.getLexeme();
		if (expr->op.type == TokenType::EQUAL)
		{
			Value val = expr->val->accept(this);
			std::get<std::shared_ptr<ToyInstance>>(object.data)->set(name, val);
			return val;
		}
		else
		{
			auto instance = std::get<std::shared_ptr<ToyInstance>>(object.data);
			Value mem = instance->get(instance, name);
			if (mem.tag == TypeTag::INSTANCE)
			{
				Value val = expr->val->accept(this);
				auto callMethod = [this](Value& a, Value& b, const std::string& name) {
					Value func = std::get<std::shared_ptr<ToyInstance>>(a.data)->get(a, name);
					Callable* callable = std::get<std::shared_ptr<Callable>>(func.data).get();
					if (func.tag == TypeTag::CALLABLE)
					{
						if(callable->arity() == 1)
							return callable->call(this, { b });
						else
						{
							err << "[ERROR] Invalid function call with invalid argument count\n";
							throw err.str();
							return Value();
						}
					}
					else
					{
						err << "[ERROR] Type " << std::get<std::shared_ptr<ToyInstance>>(a.data)->klass->name() << " does not have '" << name << "' method.\n";
						throw err.str();
						return Value();
					}
				};
				switch (expr->op.type)
				{
				case TokenType::PLUS_EQUAL:
					val = callMethod(mem, val, "__iadd__");
					break;
				case TokenType::MINUS_EQUAL:
					val = callMethod(mem, val, "__isub__");
					break;
				case TokenType::STAR_EQUAL:
					val = callMethod(mem, val, "__imul__");
					break;
				case TokenType::SLASH_EQUAL:
					val = callMethod(mem, val, "__idiv__");
					break;
				}
			}
			else if (mem.tag != TypeTag::ERR)
			{
				Value val = expr->val->accept(this);
				if (expr->op.type != TokenType::EQUAL)
				{
					if (val.tag == TypeTag::NUMBER && mem.tag == TypeTag::NUMBER)
					{
						switch (expr->op.type)
						{
						case TokenType::PLUS_EQUAL:
							val.data = std::get<double>(mem.data) + std::get<double>(val.data);
							break;
						case TokenType::MINUS_EQUAL:
							val.data = std::get<double>(mem.data) - std::get<double>(val.data);
							break;
						case TokenType::STAR_EQUAL:
							val.data = std::get<double>(mem.data) * std::get<double>(val.data);
							break;
						case TokenType::SLASH_EQUAL:
							val.data = std::get<double>(mem.data) / std::get<double>(val.data);
							break;
						}
					}
					else if (val.tag == TypeTag::STRING && mem.tag == TypeTag::STRING)
					{
						std::stringstream ss;
						ss << std::get<std::string>(mem.data) << std::get<std::string>(val.data);
						val.data = ss.str();
					}
					else
					{
						return runtimeTypeError(expr->op);
					}
				}
				std::get<std::shared_ptr<ToyInstance>>(object.data)->set(name, val);
				return val;
			}
			else
			{
				err << "[ERROR] Object does not contain the member " << name << " at line: " << expr->name.line << ".\n";
				throw err.str();
				return Value();
			}
		}
	}
	else
	{
		err << "[ERROR] Setter can only work on classes line: " << expr->name.line << ".\n";
		throw err.str();
		return Value();
	}
}

Value Interpreter::visit(ExprArrayGet* expr)
{
	Value obj = expr->object->accept(this);
	if (obj.tag == TypeTag::INSTANCE)
	{
		Value index = expr->index->accept(this);
		Value method = std::get<std::shared_ptr<ToyInstance>>(obj.data)->get(obj, "__iget__");
		Callable* callable = std::get<std::shared_ptr<Callable>>(method.data).get();
		if (callable->arity() == 1)
			return callable->call(this, { index });
		else
		{
			err << "[ERROR] Invalid function call with invalid argument count at line: " << expr->paren.line << std::endl;
			throw err.str();
			return Value();
		}
	}
	else
	{
		err << "[ERROR] Array get can only be used on an object line: " << expr->paren.line << ".\n";
		throw err.str();
		return Value();
	}
}

Value Interpreter::visit(ExprArraySet* expr)
{
	Value obj = expr->object->accept(this);
	if (obj.tag == TypeTag::INSTANCE)
	{
			Value index = expr->index->accept(this);
			Value val = expr->val->accept(this);
			Value set_method = std::get<std::shared_ptr<ToyInstance>>(obj.data)->get(obj, "__iset__");
			Callable* set_callable = std::get<std::shared_ptr<Callable>>(set_method.data).get();
			if (set_callable->arity() == 2)
			{
				if (expr->op.type != TokenType::EQUAL)
				{
					auto callMethod = [this](Value& a, Value& b, const std::string& name) {
						Value func = std::get<std::shared_ptr<ToyInstance>>(a.data)->get(a, name);
						Callable* callable = std::get<std::shared_ptr<Callable>>(func.data).get();
						if (func.tag == TypeTag::CALLABLE)
						{
							if (callable->arity() == 1)
								return callable->call(this, { b });
							else
							{
								err << "[ERROR] Invalid function call with invalid argument count\n";
								throw err.str();
								return Value();
							}
						}
						else
						{
							err << "[ERROR] Type " << std::get<std::shared_ptr<ToyInstance>>(a.data)->klass->name() << " does not have '" << name << "' method.\n";
							throw err.str();
							return Value();
						}
					};

					Value method = std::get<std::shared_ptr<ToyInstance>>(obj.data)->get(obj, "__iget__");
					Callable* callable = std::get<std::shared_ptr<Callable>>(method.data).get();
					Value getted;
					if (callable->arity() == 1)
						getted = callable->call(this, { index });
					else
					{
						err << "[ERROR] Invalid function call with invalid argument count at line: " << expr->paren.line << std::endl;
						throw err.str();
						return Value();
					}

					if (getted.tag == TypeTag::INSTANCE)
					{
						switch (expr->op.type)
						{
						case TokenType::PLUS_EQUAL:
							val = callMethod(getted, val, "__iadd__");
							break;
						case TokenType::MINUS_EQUAL:
							val = callMethod(getted, val, "__isub__");
							break;
						case TokenType::STAR_EQUAL:
							val = callMethod(getted, val, "__imul__");
							break;
						case TokenType::SLASH_EQUAL:
							val = callMethod(getted, val, "__idiv__");
							break;
						}
					}
					else if (getted.tag != TypeTag::ERR)
					{
						if (val.tag == TypeTag::NUMBER && getted.tag == TypeTag::NUMBER)
						{
							switch (expr->op.type)
							{
							case TokenType::PLUS_EQUAL:
								val.data = std::get<double>(getted.data) + std::get<double>(val.data);
								break;
							case TokenType::MINUS_EQUAL:
								val.data = std::get<double>(getted.data) - std::get<double>(val.data);
								break;
							case TokenType::STAR_EQUAL:
								val.data = std::get<double>(getted.data) * std::get<double>(val.data);
								break;
							case TokenType::SLASH_EQUAL:
								val.data = std::get<double>(getted.data) / std::get<double>(val.data);
								break;
							}
						}
						else if (val.tag == TypeTag::STRING && getted.tag == TypeTag::STRING)
						{
							std::stringstream ss;
							ss << std::get<std::string>(getted.data) << std::get<std::string>(val.data);
							val.data = ss.str();
						}
						else
						{
							return runtimeTypeError(expr->op);
						}
					}
				}

				return set_callable->call(this, { index, val });
			}
			else
			{
				err << "[ERROR] Invalid function call with invalid argument count at line: " << expr->paren.line << std::endl;
				throw err.str();
				return Value();
			}
	}
	else
	{
		err << "[ERROR] Array get can only be used on an object line: " << expr->paren.line << ".\n";
		throw err.str();
		return Value();
	}
}


Value Interpreter::visit(ExprCall* expr)
{
	Value func = expr->callee->accept(this);
	if (func.tag != TypeTag::CALLABLE)
	{
		err << "[ERROR] Invalid function call at line: " << expr->paren.line << std::endl;
		throw err.str();
		return Value();
	}
	else if (std::get<std::shared_ptr<Callable>>(func.data)->arity() == expr->args.size())
	{
		std::vector<Value> args;
		for (auto& a : expr->args)
		{
			args.push_back(a->accept(this));
		}
		return std::get<std::shared_ptr<Callable>>(func.data)->call(this, args);
	}
	else
	{
		err << "[ERROR] Invalid function call with invalid argument count at line: " << expr->paren.line << std::endl;
		throw err.str();
		return Value();
	}
}

Value Interpreter::visit(ExprLiteral* expr)
{
	return expr->value;
}

void Interpreter::visit(StmtExpr* stmt)
{
	Value a = stmt->expr->accept(this);
	// std::cout << a << std::endl;
}

void Interpreter::visit(StmtFunction* stmt)
{
	Value funcVal(std::make_shared<ToyFunction>(stmt));
	enviroment->define(stmt->name, funcVal);
}

void Interpreter::visit(StmtVarDecl* stmt)
{
	if (stmt->initVal.get())
		enviroment->define(stmt->name, stmt->initVal->accept(this));
	else
		enviroment->define(stmt->name);
}

void Interpreter::visit(StmtBlock* stmt)
{
	Enviroment* env = this->enviroment;
	this->enviroment = new Enviroment(env);

	for (auto& s : stmt->stmts)
		s->accept(this);

	delete this->enviroment;
	this->enviroment = env;
}

void Interpreter::visit(StmtIf* stmt)
{
	Value res = stmt->cond->accept(this);

	if (res.tag == TypeTag::BOOL) {
		if (std::get<bool>(res.data))
			stmt->then->accept(this);
		else if (stmt->els)
			stmt->els->accept(this);
	}
	else
	{
		err << "Invalid data type for if statement at line: " << stmt->paren.line << std::endl;
		throw err.str();
	}
}

void Interpreter::visit(StmtWhile* stmt)
{
	Value res = stmt->cond->accept(this);

	while (res.tag == TypeTag::BOOL && std::get<bool>(res.data))
	{
		stmt->then->accept(this);
		res = stmt->cond->accept(this);
	}

	if (res.tag != TypeTag::BOOL)
	{
		err << "Invalid data type for if statement at line: " << stmt->paren.line << std::endl;
		throw err.str();
	}
}

void Interpreter::visit(StmtReturn* stmt)
{
	throw stmt->expr->accept(this);
}

void Interpreter::visit(StmtClass* stmt)
{
	enviroment->define(stmt->name, Value(std::make_shared<ToyClass>(stmt->name.getLexeme(), stmt->methods)));
}
