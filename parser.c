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
		case TOKEN_IF:
			g_if();
			break;
		case TOKEN_WHILE:
			g_while();
			break;
		case TOKEN_NEWLINE:
			printf("%s NEWLINE\n", levels(m_level+1).c_str());
			advance();
			break;
		default:
			printf("%s UNKNOWN %d at %d\n", levels(m_level+1).c_str(), m_token->type, m_tokenIndex);
			advance();
			break;
	}
	m_level--;
}

void Parser::g_while()
{
	printf("%s WHILE\n", levels(++m_level).c_str());

	g_comparison();
	g_repeat();
	g_newline();

	while (m_token->type != TOKEN_ENDWHILE)
        g_statement();
    advance();

	--m_level;
}

void Parser::g_repeat()
{
	printf("%s REPEAT\n", levels(++m_level).c_str());

	if (m_token->type != TOKEN_REPEAT)
		abort("Expected 'REPEAT'");

	advance();

	--m_level;
}

void Parser::g_if()
{
	printf("%s IF\n", levels(++m_level).c_str());

	g_comparison();
	g_then();
	g_newline();

	while (m_token->type != TOKEN_ENDIF)
        g_statement();
    advance();

	--m_level;
}

void Parser::g_then()
{
	printf("%s THEN\n", levels(++m_level).c_str());
	if (m_token->type != TOKEN_THEN)
		abort("expected 'THEN'");
	advance();
	--m_level;
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
		abort("expected newline");
	}
	do advance(); while (m_token->type == TOKEN_NEWLINE);
	m_level--;
}

void Parser::g_comparison()
{
	printf("%s COMPARISON\n", levels(++m_level).c_str());

	advance();
	g_expression();
	g_comparison_operator();
	g_expression();

	while (m_token->type >= TOKEN_ARITHMETIC_OPERATORS_BEGIN && m_token->type <= TOKEN_COMPARISON_OPERATORS_END)
		g_expression();

	--m_level;
}

void Parser::g_comparison_operator()
{
	printf("%s COMPARISON_OP\n", levels(++m_level).c_str());

	// advance();
	switch (m_token->type)
	{
		case TOKEN_EQUAL:
		case TOKEN_NOT_EQUAL:
		case TOKEN_GREATER:
		case TOKEN_GE:
		case TOKEN_LESS:
		case TOKEN_LE:
			advance();
			break;
		default:
			abort("expected comparison operator");
	}

	--m_level;
}

// An expression is term (-|+) term
void Parser::g_expression()
{
	printf("%s EXPRESSION\n", levels(++m_level).c_str());

	// advance();

	g_term();
	while (m_token->type == TOKEN_ADD || m_token->type == TOKEN_SUBTRACT)
		g_term();

	m_level--;
}

// A term is unary /* unary
// Or (+= primary) /* (+= primary)
// Or (+= (Number|Identifier)) /* (+= (Number|Identifier))
void Parser::g_term()
{
	printf("%s TERM\n", levels(++m_level).c_str());

	g_unary();
	while (m_token->type == TOKEN_MULTIPLY || m_token->type == TOKEN_DIVIDE)
		g_unary();

	--m_level;
}

// A unary is += primary
void Parser::g_unary()
{
	printf("%s UNARY\n", levels(++m_level).c_str());

	if (m_token->type == TOKEN_ADD || m_token->type == TOKEN_SUBTRACT)
		advance();

	g_primary();

	--m_level;	
}

// A primary is (Number|Identifier)
void Parser::g_primary()
{
	printf("%s PRIMARY\n", levels(++m_level).c_str());

	switch (m_token->type)
	{
		case TOKEN_NUMBER:
		case TOKEN_IDENTIFIER:
			advance();
			break;
		default:
			abort("Primary: expected number or identifier");
			break;
	}

	--m_level;
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
	throw std::runtime_error("Parser aborted on token type " + std::to_string(m_token->type) + " at position " + std::to_string(m_tokenIndex) + ":\n\t" + msg);
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
