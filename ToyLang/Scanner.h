#pragma once

#include <memory.h>
#include <string>
#include <vector>

enum class TokenType
{
	// One char
	OPEN_BRACE,
	CLOSE_BRACE,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	OPEN_PAREN,
	CLOSE_PAREN,
	LESS,
	GREAT,
	DOT,
	COMMA,
	SEMI_COLON,
	COLON,
	STAR,
	PLUS,
	MINUS,
	SLASH,
	MODULUS,
	BANG,
	EQUAL,
	TILDE,

	// Two char
	LESS_EQUAL,
	GREAT_EQUAL,
	ARROW,
	BANG_EQUAL,
	EQUAL_EQUAL,
	AND,
	OR,
	PLUS_EQUAL,
	MINUS_EQUAL,
	STAR_EQUAL,
	SLASH_EQUAL,
	PLUS_PLUS,
	MINUS_MINUS,

	// Literals
	STRING_LITERAL,
	IDENTIFIER,
	NUMBER_LITERAL,
	CHAR_LITERAL,

	// Keywords
	CLASS,
	IF,
	ELSE,
	FUNC,
	SELF,
	VAR,
	WHILE,
	FOR,
	TRUE,
	FALSE,
	RETURN,

	// Util
	ERROR,
	EOF_TOKEN
};

class Token
{
public:
	const TokenType type;
	const int line;
	const char* start;
	const int length;

	Token(TokenType type, int line, const char* start, int length)
		: type(type), line(line), start(start), length(length)
	{
	}

	double getNumber()
	{
		char* lexeme = new char[length + 1];
		memcpy(lexeme, start, sizeof(char) * length);
		lexeme[length] = 0;
		return std::stod(lexeme);
	}

	std::string getString()
	{
		char* lexeme = new char[length - 1];
		memcpy(lexeme, start+1, sizeof(char) * length);
		lexeme[length-2] = 0;
		return std::string(lexeme);
	}

	std::string getLexeme()
	{
		char* lexeme = new char[length + 1];
		memcpy(lexeme, start, sizeof(char) * length);
		lexeme[length] = 0;
		return std::string(lexeme);
	}

	friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

class Scanner
{
public:
	Scanner(std::string& source);
	std::vector<Token> scanTokens();

private:
	std::string& source;
	int currentPosition;
	int startPosition;
	int line;

	Token scanToken();
	Token makeToken(TokenType type);

	std::string formatString(const char* str, size_t size);
	Token stringLiteral();
	Token identifierLiteral(char start);
	Token numberLiteral();
	Token errorToken(const char* msg);

	void skipWhitespace();

	char advance();
	char peek();
	char peekNext();
	bool match(char c);

	bool isAtEnd();
	inline bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
	inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
};

