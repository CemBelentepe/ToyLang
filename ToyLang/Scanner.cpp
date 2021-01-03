#include "Scanner.h"
#include <cstring>
#include <sstream>
#include <string_view>
#include <iomanip>

Scanner::Scanner(std::string& source)
	:source(source), startPosition(0), currentPosition(0), line(1)
{}

std::vector<Token> Scanner::scanTokens()
{
	std::vector<Token> tokens;

	do
	{
		tokens.push_back(scanToken());
	} while (tokens.back().type != TokenType::EOF_TOKEN);

	return tokens;
}

Token Scanner::scanToken()
{
	skipWhitespace();

	this->startPosition = currentPosition;
	if (isAtEnd())
		return makeToken(TokenType::EOF_TOKEN);

	char c = advance();
	switch (c)
	{
	case '{':
		return makeToken(TokenType::OPEN_BRACE);
	case '}':
		return makeToken(TokenType::CLOSE_BRACE);
	case '[':
		return makeToken(TokenType::OPEN_BRACKET);
	case ']':
		return makeToken(TokenType::CLOSE_BRACKET);
	case '(':
		return makeToken(TokenType::OPEN_PAREN);
	case ')':
		return makeToken(TokenType::CLOSE_PAREN);
	case '<':
		if (match('='))
			return makeToken(TokenType::LESS_EQUAL);
		return makeToken(TokenType::LESS);
	case '>':
		if (match('='))
			return makeToken(TokenType::GREAT_EQUAL);
		return makeToken(TokenType::GREAT);
	case '.':
		return makeToken(TokenType::DOT);
	case ',':
		return makeToken(TokenType::COMMA);
	case ';':
		return makeToken(TokenType::SEMI_COLON);
	case ':':
		return makeToken(TokenType::COLON);
	case '*':
		if (match('='))
			return makeToken(TokenType::STAR_EQUAL);
		return makeToken(TokenType::STAR);
	case '+':
		if (match('+'))
			return makeToken(TokenType::PLUS_PLUS);
		if (match('='))
			return makeToken(TokenType::PLUS_EQUAL);
		return makeToken(TokenType::PLUS);
	case '-':
		if (match('-'))
			return makeToken(TokenType::MINUS_MINUS);
		if (match('='))
			return makeToken(TokenType::MINUS_EQUAL);
		if (match('>'))
			return makeToken(TokenType::ARROW);
		return makeToken(TokenType::MINUS);
	case '/':
		if (match('='))
			return makeToken(TokenType::SLASH_EQUAL);
		return makeToken(TokenType::SLASH);
	case '%':
		return makeToken(TokenType::MODULUS);
	case '!':
		if (match('='))
			return makeToken(TokenType::BANG_EQUAL);
		return makeToken(TokenType::BANG);
	case '=':
		if (match('='))
			return makeToken(TokenType::EQUAL_EQUAL);
		return makeToken(TokenType::EQUAL);
	case '&':
		if (match('&'))
			return makeToken(TokenType::AND);
		return errorToken("Unexpected character");
	case '|':
		if (match('|'))
			return makeToken(TokenType::OR);
		return errorToken("Unexpected character");
	case '~':
		return makeToken(TokenType::TILDE);

	case '"':
		return stringLiteral();

	default:
		if (this->isAlpha(c))
			return identifierLiteral(c);
		else if (this->isDigit(c))
			return numberLiteral();
		return errorToken("Unexpected character");
	}

}

Token Scanner::makeToken(TokenType type)
{
	return Token(type, line, &source[startPosition], currentPosition - startPosition);
}

std::string Scanner::formatString(const char* str, size_t size)
{
	std::stringstream ss;
	size_t i = 0;
	ss << str[i++];
	while (i < size)
	{
		if (str[i] == '\\')
		{
			switch (str[++i])
			{
			case 'n':
				ss << '\n';
				break;
			case 't':
				ss << '\t';
				break;
			case '\\':
				ss << '\\';
				break;
			case 'r':
				ss << '\r';
				break;
			case 'b':
				ss << '\b';
				break;
			case 'a':
				ss << '\a';
				break;
			case 'v':
				ss << '\v';
				break;
			case '0':
				ss << '\0';
				break;
			case '\'':
				ss << '\'';
				break;
			default:
				break;
			}
			i++;
		}
		else
		{
			ss << str[i++];
		}
	}
	ss << str[i++];
	return ss.str();
}


Token Scanner::stringLiteral()
{
	while (peek() != '"' && !this->isAtEnd())
	{
		advance();
	}

	if (isAtEnd())
		return errorToken("Unterminated string.");

	advance();
	std::string formattedString = formatString(&source[startPosition], currentPosition - startPosition - 1);
	char* str = new char[formattedString.size() + 1];
	formattedString.copy(str, formattedString.size());
	return Token(TokenType::STRING_LITERAL, line, str, formattedString.size());
}

Token Scanner::identifierLiteral(char start)
{
	std::stringstream lexeme;
	lexeme << start;
	while ((this->isAlpha(peek()) || this->isDigit(peek())))
	{
		lexeme << advance();
	}

	if (lexeme.str() == "class")
		return makeToken(TokenType::CLASS);
	if (lexeme.str() == "if")
		return makeToken(TokenType::IF);
	if (lexeme.str() == "else")
		return makeToken(TokenType::ELSE);
	if (lexeme.str() == "func")
		return makeToken(TokenType::FUNC);
	if (lexeme.str() == "self")
		return makeToken(TokenType::SELF);
	if (lexeme.str() == "var")
		return makeToken(TokenType::VAR);
	if (lexeme.str() == "while")
		return makeToken(TokenType::WHILE);
	if (lexeme.str() == "for")
		return makeToken(TokenType::FOR);
	if (lexeme.str() == "true")
		return makeToken(TokenType::TRUE);
	if (lexeme.str() == "false")
		return makeToken(TokenType::FALSE);
	if (lexeme.str() == "return")
		return makeToken(TokenType::RETURN);

	return makeToken(TokenType::IDENTIFIER);
}

Token Scanner::numberLiteral()
{
	while (this->isDigit(peek()))
	{
		advance();
	}
	if (match('.'))
	{
		while (this->isDigit(peek()))
		{
			advance();
		}
	}
	return makeToken(TokenType::NUMBER_LITERAL);
}

Token Scanner::errorToken(const char* msg)
{
	return Token(TokenType::ERROR, line, msg, strlen(msg));
}

void Scanner::skipWhitespace()
{
	while (true)
	{
		char c = peek();
		switch (c)
		{
		case ' ':
		case '\t':
		case '\r':
			advance();
			break;
		case '\n':
			this->line++;
			advance();
			break;

		case '/':
			if (peekNext() == '/')
			{
				while (peek() != '\n' && !this->isAtEnd())
					advance();
				skipWhitespace();
			}
			else if (peekNext() == '*')
			{
				while (!(peek() == '*' && peekNext() == '/') && !this->isAtEnd())
				{
					char x = advance();
					if (x == '\n')
						this->line++;
				}
				this->currentPosition += 2;
				skipWhitespace();
			}
			break;
		default:
			return;
		}
	}
}

char Scanner::advance()
{
	return source[this->currentPosition++];
}

char Scanner::peek()
{
	return source[this->currentPosition];
}

char Scanner::peekNext()
{
	if (this->isAtEnd())
		return '\0';
	return source[this->currentPosition + 1];
}

bool Scanner::match(char c)
{
	if (this->isAtEnd() || source[this->currentPosition] != c)
		return false;

	this->currentPosition++;
	return true;
}

bool Scanner::isAtEnd()
{
	return this->currentPosition == source.length();
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
	os << "Token length: " << token.length << ", line: " << std::setw(2) << (int)token.line << ", type: " << std::setw(2) << (int)token.type << ", lexeme: " << std::string_view(token.start, token.length);
	return os;
}
