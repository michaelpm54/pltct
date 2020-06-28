/* SPDX-License-Identifier: GPLv3-or-later */

#ifndef LEXER_H
#define LEXER_H

typedef enum TokenType_s
{
	TOKEN_NONE,
	TOKEN_UNKNOWN,
	TOKEN_EOF,
	TOKEN_NUMBER,
	TOKEN_IDENTIFIER,
	TOKEN_KEYWORD,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
	TOKEN_ADD,
	TOKEN_SUBTRACT,
	TOKEN_LESS,
	TOKEN_GREATER,
	TOKEN_GE,
	TOKEN_LE,
	TOKEN_EQUAL,
	TOKEN_ASSIGN,
	TOKEN_STRING,
	TOKEN_NEWLINE,
	TOKEN_ARITHMETIC_OPERATORS_BEGIN = TOKEN_MULTIPLY,
	TOKEN_ARITHMETIC_OPERATORS_END = TOKEN_EQUAL,
	TOKEN_COMPARISON_OPERATORS_BEGIN = TOKEN_LESS,
	TOKEN_COMPARISON_OPERATORS_END = TOKEN_ASSIGN,
} TokenType;

typedef struct Token_s
{
	TokenType type;
	char *text;
} Token;

typedef enum StopReason_e
{
	STOP_NONE,
	STOP_EOF,
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
	char *buf;
	StopReason stop;
	char *errMsg;
	int line;
	int column;
	int numTokens;
	int maxTokens;
	Token *tokens;
} Lexer;

int lexer_init(Lexer *l, const char *filename);
int lexer_run(Lexer *l);
void lexer_enumerate(Lexer *l, FILE *out);
void lexer_free(Lexer *l);

#endif // LEXER_H
