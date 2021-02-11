#pragma once
#include <unordered_map>

#include "Value.h"
#include "Scanner.h"

class Enviroment
{
public:
	Enviroment* closing;
	std::unordered_map<std::string, Value> vars;
	
	Enviroment(Enviroment* closing = nullptr)
		: closing(closing)
	{ }

	void define(Token name, Value val = Value())
	{
		std::string sName = name.getLexeme();
		auto it = vars.find(sName);
		if (it == vars.end())
			vars[sName] = val;
		else
			std::cout << "[Error] Variable '" << name.getLexeme() << "' is a duplicate definition, line: " << name.line << std::endl;
	}

	void define(std::string name, Value val = Value())
	{
		auto it = vars.find(name);
		if (it == vars.end())
			vars[name] = val;
		else
			std::cout << "[Error] Variable '" << name << "' is a duplicate definition"<< std::endl;
	}

	Value getVar(std::string name)
	{
		Enviroment* look = this;
		while (look)
		{
			auto it = look->vars.find(name);
			if (it == look->vars.end())
				look = look->closing;
			else
				return it->second;
		}
		std::cout << "[Error] Variable '" << name << "' does not exists." << std::endl;
		return Value();
	}

	Value setVar(std::string name, Value set)
	{
		Enviroment* look = this;
		while (look)
		{
			auto it = look->vars.find(name);
			if (it == look->vars.end())
				look = look->closing;
			else
				return it->second = set; // TODO OK?
		}
		std::cout << "[Error] Variable '" << name << "' does not exists." << std::endl;
		return Value();
	}
};