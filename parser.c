/* SPDX-License-Identifier: GPLv3-or-later */

#include "parser.h"

#include <stdexcept>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

std::string levels(int level)
{
	return std::string(level, '-');
}

void Parser::run(const std::vector<Token> &tokens)
{
	if (tokens.empty())
		return;

	m_tokens = tokens;
	m_token = &tokens[0];

	g_program();
}

void Parser::g_program()
{
	puts("PROGRAM");
	while (m_token->type != TOKEN_EOF)
		g_statement();
	puts("DONE");
}

void Parser::g_statement()
{
	printf("%s STATEMENT\n", levels(++m_level).c_str());
	switch (m_token->type)
	{
		case TOKEN_PRINT:
			g_print();
			break;
		case TOKEN_INPUT:
			g_input();
			break;
		case TOKEN_LET:
			g_let();
			break;
		default:
			printf("%s UNKNOWN\n", levels(++m_level).c_str());
			m_level--;
			advance();
			break;
	}
	m_level--;
}

void Parser::g_print()
{
	printf("%s PRINT\n", levels(++m_level).c_str());
	advance();
	if (m_token->type == TOKEN_STRING)
		g_string();
	else
		g_expression();
	g_newline();
	m_level--;
}

void Parser::g_input()
{
	printf("%s INPUT\n", levels(++m_level).c_str());
	if (next_token() && next_token()->type != TOKEN_IDENTIFIER)
		abort("bad token type for input");
	advance();
	g_identifier();
	g_newline();
	m_level--;
}

void Parser::g_identifier()
{
	printf("%s IDENTIFIER\n", levels(++m_level).c_str());
	advance();
	m_level--;
}

void Parser::g_newline()
{
	printf("%s NEWLINE\n", levels(++m_level).c_str());
	if (m_token->type != TOKEN_NEWLINE)
	{
		printf("token type %d\n", m_token->type);
		abort("expected newline");
	}
	do advance(); while (m_token->type == TOKEN_NEWLINE);
	m_level--;
}

void Parser::g_expression()
{
	printf("%s EXPRESSION\n", levels(++m_level).c_str());

	advance();

	switch (m_token->type)
	{
		case TOKEN_IDENTIFIER:
			g_identifier();
			break;
		case TOKEN_STRING:
			g_string();
			break;
		default:
			abort("bad token type in expression: " + std::to_string(m_token->type));
	}

	m_level--;
}

void Parser::g_string()
{
	printf("%s STRING\n", levels(++m_level).c_str());
	advance();
	m_level--;
}

void Parser::g_let()
{
	printf("%s LET\n", levels(++m_level).c_str());
	advance();

	switch (m_token->type)
	{
		case TOKEN_IDENTIFIER:
			g_identifier();
			break;
		default:
			abort("bad token type for let: " + std::to_string(m_token->type));
			break;
	}

	g_assign();
	g_expression();
	g_newline();

	m_level--;
}

void Parser::g_assign()
{
	printf("%s ASSIGN\n", levels(++m_level).c_str());

	if (m_token->type != TOKEN_ASSIGN)
		abort("expected assignment operator");

	advance();

	m_level--;
}

void Parser::abort(const std::string &msg)
{
	throw std::runtime_error("Parser aborted: " + msg);
}

Token *Parser::next_token()
{
	if (m_tokenIndex >= m_tokens.size())
		return nullptr;
	return &m_tokens[m_tokenIndex+1];
}

Token *Parser::prev_token()
{
	if (m_tokenIndex == 0)
		return nullptr;
	return &m_tokens[m_tokenIndex-1];
}

void Parser::advance()
{
	if (m_tokenIndex >= m_tokens.size())
		abort("exceeded num tokens");
	m_token = &m_tokens[++m_tokenIndex];
}
