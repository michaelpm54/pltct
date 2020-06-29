/* SPDX-License-Identifier: GPLv3-or-later */

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

#include "token.h"

typedef enum StopReason_e
{
	STOP_NONE,
	STOP_UNKNOWN_TOKEN,
	STOP_INVALID_NUMBER,
	STOP_BAD_FILE,
	STOP_PEEK_EOF,
	STOP_INVALID_STRING,
} StopReason;

typedef struct Lexer_s
{
	int pos;
	long unsigned int size;
	char c;
	std::string buf;
	StopReason stop;
	int line;
	int column;
	int numTokens;
	int maxTokens;
	std::vector<Token> tokens;
} Lexer;

void lexer_init(Lexer *l, const char *filename);
void lexer_run(Lexer *l);
void lexer_enumerate(Lexer *l, FILE *out);

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

class CLexer
{
public:
	std::vector<Token> run(const std::string &input);
	void enumerate(FILE *out);

private:
	Token get_token();
	void advance();
	char peek();

	void get_number(Token &token);
	void get_string(Token &token);

	std::string token_type(const Token &token)
	{
		switch (token.type)
		{
			case TOKEN_KEYWORDS_BEGIN:
			case TOKEN_KEYWORDS_END:
				return "Keyword";
			case TOKEN_IDENTIFIER:
				return "Identifier";
			case TOKEN_STRING:
				return "String";
			case TOKEN_NEWLINE:
				return "Newline";
			case TOKEN_ASSIGN:
				return "Assign";
			case TOKEN_LESS:
				return "Less than";
			case TOKEN_GREATER:
				return "Greater than";
			case TOKEN_LE:
				return "Less than or equal to";
			case TOKEN_GE:
				return "Greater than or equal to";
			case TOKEN_ADD:
				return "Add";
			case TOKEN_SUBTRACT:
				return "Subtract";
			case TOKEN_NUMBER:
				return "Number";
			case TOKEN_EOF:
				return "EOF";
			case TOKEN_NONE:
				return "None";
			case TOKEN_UNKNOWN:
				return "Unknown";
			default:
				return "<unhandled>";
		}
	}

	std::string get_stop_reason(StopReason r)
	{
		switch (r)
		{
			case STOP_NONE:
				return "None";
			case STOP_UNKNOWN_TOKEN:
				return "Unknown token";
			case STOP_INVALID_NUMBER:
				return "Invalid number";
			case STOP_BAD_FILE:
				return "Bad file";
			case STOP_PEEK_EOF:
				return "Peek EOF";
			case STOP_INVALID_STRING:
				return "Invalid string";
			default:
				return "Unknown";
		}
	}


	template<typename ... Args>
	void abort(StopReason stop, const char * const format, Args ... args)
	{
		throw std::runtime_error(string_format("%s: line %d column %d: '%s'",
			get_stop_reason(stop).c_str(), m_line, m_column, string_format(format, args ...).c_str()));
	}

private:
	char m_c{0};
	int m_pos{0};
	int m_line{1};
	int m_column{1};
	int m_numTokens{0};
	int m_maxTokens{0};
	int m_numTokenChunks{1};
	StopReason m_stop;
	std::string m_input;
	std::vector<Token> m_tokens;
};


#endif // LEXER_H
