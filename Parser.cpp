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
		try
		{
			root.push_back(std::move(decleration()));
		}
		catch (bool err)
		{
			hadError = true;
		}
	}

	return root;
}

std::unique_ptr<Stmt> Parser::decleration()
{
	Token token = advance();

	if (token.type == TokenType::FUNC)
	{
		return function("function");
	}
	else if (token.type == TokenType::CLASS)
	{
		return classDecl();
	}
	else if (token.type == TokenType::VAR)
	{
		return varDecl();
	}

	std::cout << "[ERROR] Invalid token '" << token.getLexeme() << "' at line: " << token.line << std::endl;
	return nullptr;
}

std::unique_ptr<StmtFunction> Parser::function(std::string type)
{
	Token name = advance();
	std::vector<Token> params;
	consume(TokenType::OPEN_PAREN, "Expect '(' after a " + type + " name");
	if (peek().type != TokenType::CLOSE_PAREN)
	{
		params.push_back(advance());
	}
	while (!match(TokenType::CLOSE_PAREN))
	{
		consume(TokenType::COMMA, "Expect ',' between parameters."); // consume ','
		params.push_back(advance());
	}

	consume(TokenType::OPEN_BRACE, "Expect '{' at " + type + " start.");

	std::vector<std::unique_ptr<Stmt>> body;
	while (!match(TokenType::CLOSE_BRACE))
	{
		body.push_back(std::move(statement()));
	}

	return std::make_unique<StmtFunction>(name, std::move(body), params);
}

std::unique_ptr<StmtVarDecl> Parser::varDecl()
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

std::unique_ptr<StmtClass> Parser::classDecl()
{
	Token name = advance();
	consume(TokenType::OPEN_BRACE, "Expect '{' after class name.");
	std::vector<std::unique_ptr<StmtFunction>> methods;

	while (!match(TokenType::CLOSE_BRACE))
	{
		methods.push_back(std::move(function("method")));
	}

	return std::make_unique<StmtClass>(name, std::move(methods));
}

std::unique_ptr<Stmt> Parser::statement()
{
	if (peek().type == TokenType::VAR)
	{
		advance();
		return std::move(varDecl());
	}
	else if (peek().type == TokenType::IF)
	{
		advance();
		return std::move(ifStatement());
	}
	else if (peek().type == TokenType::WHILE)
	{
		advance();
		return std::move(whileStatement());
	}
	else if (peek().type == TokenType::FOR)
	{
		advance();
		return std::move(forStatement());
	}
	else if (peek().type == TokenType::OPEN_BRACE)
	{
		advance();
		return std::move(block());
	}
	else if (peek().type == TokenType::RETURN)
	{
		advance();
		auto expr = parseExpr();
		consume(TokenType::SEMI_COLON, "Expect ';' after a return statement.");
		return std::make_unique<StmtReturn>(std::move(expr));
	}
	else if (peek().type != TokenType::SEMI_COLON)
	{
		auto expr = parseExpr();
		consume(TokenType::SEMI_COLON, "Expect ';' after an expression statement.");
		return std::unique_ptr<Stmt>(new StmtExpr(std::move(expr)));
	}
	else
	{
		return std::make_unique<StmtBlock>(std::vector<std::unique_ptr<Stmt>>());
	}
}

std::unique_ptr<StmtBlock> Parser::block()
{
	std::vector<std::unique_ptr<Stmt>> stmts;
	while (!match(TokenType::CLOSE_BRACE))
	{
		stmts.push_back(statement());
	}
	return std::make_unique<StmtBlock>(std::move(stmts));
}

std::unique_ptr<StmtIf> Parser::ifStatement()
{
	consume(TokenType::OPEN_PAREN, "Expect '(' after 'if'.");
	Token paren = consumed();
	std::unique_ptr<Expr> cond = parseExpr();
	consume(TokenType::CLOSE_PAREN, "Expect ')' after if condition.");

	std::unique_ptr<Stmt> then = statement();
	std::unique_ptr<Stmt> els = nullptr;
	if (match(TokenType::ELSE))
		els = statement();

	return std::make_unique<StmtIf>(std::move(cond), paren, std::move(then), std::move(els));
}

std::unique_ptr<StmtWhile> Parser::whileStatement()
{
	consume(TokenType::OPEN_PAREN, "Expect '(' after 'while'.");
	Token paren = consumed();
	std::unique_ptr<Expr> cond = parseExpr();
	consume(TokenType::CLOSE_PAREN, "Expect ')' after while condition.");

	std::unique_ptr<Stmt> then = statement();

	return std::make_unique<StmtWhile>(std::move(cond), paren, std::move(then));
}

std::unique_ptr<StmtBlock> Parser::forStatement()
{
	consume(TokenType::OPEN_PAREN, "Expect '(' after 'for'.");
	Token paren = consumed();

	std::unique_ptr<Stmt> decl = nullptr;
	if (match(TokenType::VAR))
		decl = varDecl();
	else if (!match(TokenType::SEMI_COLON))
	{
		auto expr = parseExpr();
		consume(TokenType::SEMI_COLON, "Expect ';' after decleration statement of 'for'.");
		decl = std::make_unique<StmtExpr>(std::move(expr));
	}

	std::unique_ptr<Expr> cond = std::make_unique<ExprLiteral>(Value(true));
	if (!match(TokenType::SEMI_COLON))
	{
		cond = parseExpr();
		consume(TokenType::SEMI_COLON, "Expect ';' after an expression statement of 'for'.");
	}

	std::unique_ptr<Expr> inc = nullptr;
	if (!match(TokenType::CLOSE_PAREN))
	{
		inc = parseExpr();
		consume(TokenType::CLOSE_PAREN, "Expect ')' at the end of for statement conditions.");
	}

	std::unique_ptr<Stmt> forBody = statement();
	if (inc.get() != nullptr)
	{
		std::vector<std::unique_ptr<Stmt>> bdy;
		bdy.push_back(std::move(forBody));
		bdy.push_back(std::make_unique<StmtExpr>(std::move(inc)));
		forBody = std::make_unique<StmtBlock>(std::move(bdy));
	}

	std::unique_ptr<StmtWhile> whilePart = std::make_unique<StmtWhile>(std::move(cond), paren, std::move(forBody));

	std::vector<std::unique_ptr<Stmt>> bodyStmts;
	if (decl.get() != nullptr)
		bodyStmts.push_back(std::move(decl));
	bodyStmts.push_back(std::move(whilePart));

	return std::make_unique<StmtBlock>(std::move(bodyStmts));
}

std::unique_ptr<Expr> Parser::parseExpr()
{
	return std::move(assignment());
}

std::unique_ptr<Expr> Parser::assignment()
{
	std::unique_ptr<Expr> expr = logic_or();

	if (match({ TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> asgn = std::move(parseExpr());

		if (expr->instance == ExprType::VariableGet)
		{
			Token name = ((ExprVariableGet*)expr.get())->name;
			return std::make_unique<ExprVariableSet>(name, std::move(asgn), op);
		}
		else if (expr->instance == ExprType::MemberGet)
		{
			ExprMemberGet* get = (ExprMemberGet*)expr.get();
			return std::make_unique<ExprMemberSet>(get->name, std::move(get->object), std::move(asgn), op);
		}
		else if (expr->instance == ExprType::ArrayGet)
		{
			ExprArrayGet* get = (ExprArrayGet*)expr.get();
			return std::make_unique<ExprArraySet>(get->paren, std::move(get->object), std::move(get->index), std::move(asgn), op);
		}
		else
		{
			return this->errorAtToken("[ERROR] Invalid assignment target");
		}
	}

	return std::move(expr);
}

std::unique_ptr<Expr> Parser::logic_or()
{
	std::unique_ptr<Expr> lhs = logic_and();
	while (match(TokenType::OR))
	{
		Token op = consumed();
		std::unique_ptr<Expr> rhs = logic_and();
		lhs = std::make_unique<ExprBinary>(std::move(lhs), std::move(rhs), op);
	}

	return std::move(lhs);
}

std::unique_ptr<Expr> Parser::logic_and()
{
	std::unique_ptr<Expr> lhs = equality();
	while (match(TokenType::AND))
	{
		Token op = consumed();
		std::unique_ptr<Expr> rhs = equality();
		lhs = std::make_unique<ExprBinary>(std::move(lhs), std::move(rhs), op);
	}

	return std::move(lhs);
}

std::unique_ptr<Expr> Parser::equality()
{
	std::unique_ptr<Expr> lhs = comparison();
	while (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> rhs = comparison();
		lhs = std::make_unique<ExprBinary>(std::move(lhs), std::move(rhs), op);
	}

	return std::move(lhs);
}

std::unique_ptr<Expr> Parser::comparison()
{
	std::unique_ptr<Expr> lhs = addition();
	while (match({ TokenType::LESS, TokenType::GREAT, TokenType::LESS_EQUAL, TokenType::GREAT_EQUAL }))
	{
		Token op = consumed();
		std::unique_ptr<Expr> rhs = addition();
		lhs = std::make_unique<ExprBinary>(std::move(lhs), std::move(rhs), op);
	}

	return std::move(lhs);
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

	while (true)
	{
		if (match(TokenType::OPEN_PAREN)) {
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
		else if (match(TokenType::DOT))
		{
			consume(TokenType::IDENTIFIER, "Expect an identifier as a member.");
			Token mem = consumed();
			expr = std::make_unique<ExprMemberGet>(mem, std::move(expr));
		}
		else if (match(TokenType::OPEN_BRACKET))
		{
			Token paren = consumed();
			std::unique_ptr<Expr> index = parseExpr();
			consume(TokenType::CLOSE_BRACKET, "Expect ']' after an index of '['.");
			expr = std::make_unique<ExprArrayGet>(paren, std::move(expr), std::move(index));
		}
		else
		{
			break;
		}
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
	case TokenType::SELF:
		return std::make_unique<ExprVariableGet>(token);
	default:
		return errorAtToken("Invalid identifier.");
	}
}

void Parser::consume(TokenType type, std::string msg)
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
	throw true;
}