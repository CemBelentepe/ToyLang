#include "Parser.h"
#include "AST.h"
#include "Value.h"

Parser::Parser(std::vector<Token>& tokens)
	: tokens(tokens), currentToken(0)
{}

std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
	std::vector<std::unique_ptr<Stmt>> root;

	while (!match(TokenType::EOF_TOKEN))
	{
		root.push_back(std::move(decleration()));
	}

	return root;
}

std::unique_ptr<Stmt> Parser::decleration()
{
	Token token = advance();

	if (token.type == TokenType::FUNC)
	{
		return std::move(function());
	}
	else if (token.type == TokenType::VAR)
	{
		return std::move(varDecl());
	}

	std::cout << "[ERROR] Invalid token '" << token.getLexeme() << "' at line: " << token.line << std::endl;
	return nullptr;
}

std::unique_ptr<Stmt> Parser::function()
{
	Token name = advance();
	std::vector<Token> params;
	consume(TokenType::OPEN_PAREN, "Expect '(' after a function name");
	if (peek().type != TokenType::CLOSE_PAREN)
	{
		params.push_back(advance());
	}
	while (!match(TokenType::CLOSE_PAREN))
	{
		consume(TokenType::COMMA, "Expect ',' between parameters."); // consume ','
		params.push_back(advance());
	}

	consume(TokenType::OPEN_BRACE, "Expect '{' at function start.");

	std::vector<std::unique_ptr<Stmt>> body;
	while (!match(TokenType::CLOSE_BRACE))
	{
		body.push_back(std::move(statement()));
	}

	return std::unique_ptr<Stmt>(new StmtFunction(name, std::move(body), params));
}

std::unique_ptr<Stmt> Parser::varDecl()
{
	Token name = advance();
	std::unique_ptr<Expr> init(nullptr);
	if (match(TokenType::EQUAL))
	{
		init = std::move(parseExpr());
	}
	
	consume(TokenType::SEMI_COLON, "Expect ';' after variable decleration.");
	return std::make_unique<StmtVarDecl>(name, std::move(init));
}

std::unique_ptr<Stmt> Parser::statement()
{
	if (peek().type == TokenType::VAR)
	{
		advance();
		return std::move(varDecl());
	}
	else
	{
		auto expr = parseExpr();
		consume(TokenType::SEMI_COLON, "Expect ';' after an expression statement.");
		return std::unique_ptr<Stmt>(new StmtExpr(std::move(expr)));
	}
}

std::unique_ptr<Expr> Parser::parseExpr()
{
	return std::move(assignment());
}

std::unique_ptr<Expr> Parser::assignment()
{
	std::unique_ptr<Expr> expr = addition();

	if (match({ TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> asgn = std::move(parseExpr());

		if (expr->instance == ExprType::VariableGet)
		{
			Token name = ((ExprVariableGet*)expr.get())->name;
			return std::make_unique<ExprVariableSet>(name, std::move(asgn), op);
		}
		else
		{
			return this->errorAtToken("[ERROR] Invalid assignment target");
		}
	}

	return std::move(expr);
}

std::unique_ptr<Expr> Parser::addition()
{
	std::unique_ptr<Expr> lhs = multiplication();
	while (match({ TokenType::PLUS, TokenType::MINUS }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> rhs = multiplication();
		lhs = std::make_unique<ExprBinary>(std::move(lhs), std::move(rhs), op);
	}

	return std::move(lhs);
}

std::unique_ptr<Expr> Parser::multiplication()
{
	std::unique_ptr<Expr> lhs = unary();
	while (match({ TokenType::STAR, TokenType::SLASH }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> rhs = unary();
		lhs = std::make_unique<ExprBinary>(std::move(lhs), std::move(rhs), op);
	}

	return std::move(lhs);
}

std::unique_ptr<Expr> Parser::unary()
{
	if (match({ TokenType::MINUS, TokenType::BANG }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> expr = unary();
		expr = std::make_unique<ExprUnary>(std::move(expr), op);
		return std::move(expr);
	}
	else
		return std::move(call());
}

std::unique_ptr<Expr> Parser::call()
{
	std::unique_ptr<Expr> expr = primary();

	while (match(TokenType::OPEN_PAREN))
	{
		std::vector<std::unique_ptr<Expr>> args;
		std::unique_ptr<Expr> callee = std::move(expr);
		Token paren = consumed();
		if (peek().type != TokenType::CLOSE_PAREN)
		{
			do
			{
				args.push_back(parseExpr());
			} while (match(TokenType::COMMA));
		}
		consume(TokenType::CLOSE_PAREN, "Expect ')' after arguments.");
		expr = std::make_unique<ExprCall>(std::move(callee), std::move(args), paren);
	}

	return std::move(expr);
}

std::unique_ptr<Expr> Parser::primary()
{
	Token token = advance();

	switch (token.type)
	{
	case TokenType::TRUE:
		return std::make_unique<ExprLiteral>(true);
	case TokenType::FALSE:
		return std::make_unique<ExprLiteral>(false);
	case TokenType::NUMBER_LITERAL:
		return std::make_unique<ExprLiteral>(token.getNumber());
	case TokenType::STRING_LITERAL:
		return std::make_unique<ExprLiteral>(token.getString());
	case TokenType::OPEN_PAREN:
	{
		std::unique_ptr<Expr> expr = parseExpr();
		if (match(TokenType::CLOSE_PAREN))
			return std::move(expr);
		else
			return errorAtToken("Expect ')' after a grouping expression.");
	}
	case TokenType::IDENTIFIER:
		return std::make_unique<ExprVariableGet>(token);
	default:
		return errorAtToken("Invalid identifier.");
	}
}

void Parser::consume(TokenType type, const char* msg)
{
	if (tokens[currentToken].type == type)
	{
		advance();
	}
	else
	{
		errorAtToken(msg);
	}
}

void Parser::panic()
{
	TokenType type = tokens[currentToken].type;
	while (type != TokenType::EOF_TOKEN)
	{
		if (type == TokenType::SEMI_COLON)
		{
			advance();
			return;
		}

		switch (type)
		{
		case TokenType::CLASS:
		case TokenType::IF:
		case TokenType::VAR:
		case TokenType::FUNC:
		case TokenType::WHILE:
		case TokenType::FOR:
		case TokenType::RETURN:
			return;

		default:
			break;
		}

		type = advance().type;
	}
}