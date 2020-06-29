/* SPDX-License-Identifier: GPLv3-or-later */

#include "parser.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

void parser_g_program(Parser *p);
void parser_g_statement(Parser *p);
void parser_g_print(Parser *p);
void parser_g_expression(Parser *p);
void parser_g_newline(Parser *p);

void parser_abort(Parser *p, const char * const fmt, ...);
Token *parser_advance(Parser *p);
void parser_append(Parser *p, const char *s);

static jmp_buf buf;

void parser_abort(Parser *p, const char * const fmt, ...)
{
	if (fmt && fmt[0] != '\0')
	{
		char *err = static_cast<char*>(malloc(128));
	
		va_list args;
    	va_start(args, fmt);
		vsnprintf(err, 128, fmt, args);
    	va_end(args);	

    	fprintf(stderr, "%s\n", err);
	}
	longjmp(buf, 1);
}

int parser_init(Parser *p, Token *tokens, int numTokens)
{
	if (setjmp(buf))
		return EXIT_FAILURE;

	p->numTokens = numTokens;
	p->tokens = static_cast<Token*>(malloc(numTokens * sizeof(Token)));
	p->i = 0;
	if (!p->tokens)
		return EXIT_FAILURE;
	memcpy(p->tokens, tokens, numTokens * sizeof(Token));
	return EXIT_SUCCESS;
}

int parser_run(Parser *p)
{
	parser_g_program(p);
	return EXIT_SUCCESS;
}

void parser_enumerate(Parser *p, FILE *out)
{
	// fprintf(out, "Parser:\n");
}

void parser_free(Parser *p)
{
	free(p->tokens);
}

void parser_g_program(Parser *p)
{
	if (!p->numTokens)
		return;

	parser_append(p, "PROGRAM");
	while (p->tokens[p->i].type != TOKEN_EOF)
	{
		parser_g_statement(p);
	}

	printf("DONE\n");
}

void parser_g_statement(Parser *p)
{
	printf("STMT-");
	switch (p->tokens[p->i].type)
	{
		case TOKEN_PRINT:
			parser_g_print(p);
			break;
		default:
			printf("UNK\n");
			break;
	}
}

void parser_g_print(Parser *p)
{
	printf("PRINT\n");
	Token *t = parser_advance(p);
	if (t->type == TOKEN_STRING)
		parser_advance(p);
	else
		parser_g_expression(p);
	parser_g_newline(p);
}

void parser_g_expression(Parser *p)
{
	printf("EXPRESSION\n");
	// parser
}

void parser_g_newline(Parser *p)
{
	printf("NEWLINE\n");
	if (p->tokens[p->i].type != TOKEN_NEWLINE)
		parser_abort(p, "Expected newline");
	parser_advance(p);
	while (p->tokens[p->i].type == TOKEN_NEWLINE)
		parser_advance(p);
}

void parser_append(Parser *p, const char *s)
{
	fprintf(stdout, "%s\n", s);
}

Token *parser_advance(Parser *p)
{
	p->i++;
	if (p->i >= p->numTokens)
		p->i = p->numTokens-1;

	return &p->tokens[p->i];
}
