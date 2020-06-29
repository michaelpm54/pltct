/* SPDX-License-Identifier: GPLv3-or-later */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "token.h"

typedef struct Parser_s
{
	int numTokens;
	Token *tokens;
} Parser;

int parser_init(Parser *p, Token *tokens, int numTokens);
int parser_run(Parser *p);
void parser_enumerate(Parser *p, FILE *out);
void parser_free(Parser *p);

#endif // PARSER_H
