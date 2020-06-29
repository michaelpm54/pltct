#include "parser.h"

#include <stdlib.h>
#include <string.h>

void parser_g_program(Parser *p);
void parser_g_statement(Parser *p);

void parser_append(Parser *p, const char *s);

int parser_init(Parser *p, Token *tokens, int numTokens)
{
	p->numTokens = numTokens;
	p->tokens = malloc(numTokens * sizeof(Token));
	if (!p->tokens)
		return EXIT_FAILURE;
	memcpy(p->tokens, tokens, numTokens * sizeof(Token));
	return EXIT_SUCCESS;
}

int parser_run(Parser *p)
{
	fprintf(stdout, "Parser\n");
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
	parser_append(p, "Program");
}

void parser_append(Parser *p, const char *s)
{
	fprintf(stdout, "%s\n", s);
}
