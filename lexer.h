/* SPDX-License-Identifier: GPLv3-or-later */

#ifndef LEXER_H
#define LEXER_H

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
