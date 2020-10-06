/* SPDX-License-Identifier: GPLv3-or-later */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

struct Token;

class Parser
{
public:
	void run(const std::vector<Token> &tokens);

private:
	void abort(const std::string &msg);
	void advance();
	Token *next_token();
	Token *prev_token();

private:
	void g_program();
	void g_statement();
	void g_expression();
	void g_newline();
	void g_string();
	void g_identifier();
	void g_comparison();
	void g_term();
	void g_unary();
	void g_primary();

	// Operators
	void g_assign();
	void g_comparison_operator();

	// Keywords
	void g_print();
	void g_input();
	void g_let();
	void g_if();
	void g_then();
	void g_while();
	void g_repeat();

private:
	std::vector<Token> m_tokens;
	const Token *m_token{nullptr};
	int m_tokenIndex{0};
	int m_level{0};
};

#endif // PARSER_H
