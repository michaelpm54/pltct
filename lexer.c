/* SPDX-License-Identifier: GPLv3-or-later */

/* TODO:
	- Assign ascii values to the enum to make comparison faster.
	- Use unicode instead?
 	- Disallow whitespace in strings
 	- Ignore comments
*/

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "lexer.h"

#define OUTPUT_TEXT_SIZE 128
#define LEXER_EOF '\0'
#define ERR_MAX_LEN 128
#define TOKEN_CHUNK_ALLOC_SIZE 32768 // number of tokens per allocated chunk

const char * const kKeywords[] =
{
	"WHILE",
	"ENDWHILE",
	"LET",
	"PRINT",
	"INPUT",
	"REPEAT",
};

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

bool is_keyword(std::string_view str)
{
	for (int i = 0; i < COUNT_OF(kKeywords); i++)
	{
		if (str == std::string(kKeywords[i]))
		{
			return true;
		}
	}
	return false;
}

void CLexer::get_string(Token &token)
{
	token.type = TOKEN_STRING;

	advance();
	while (m_c != '"' && m_c != LEXER_EOF)
	{
		if (m_c == '\\' || m_c == '%' || m_c == '\r' || m_c == '\n' || m_c == '\t')
			abort(STOP_INVALID_STRING, "%c", m_c);
		advance();
	}
	advance();
}

void CLexer::get_number(Token &token)
{
	token.type = TOKEN_NUMBER;

	bool havePoint = false;
	bool haveNumberAfterPoint = false;
	do {
		advance();

		if (isdigit(m_c))
		{
			if (havePoint && !haveNumberAfterPoint)
				haveNumberAfterPoint = true;
		}
		else if (m_c == '.')
		{
			if (havePoint)
			{
				abort(STOP_INVALID_NUMBER, "Multiple decimal points.");
			}
			else
				havePoint = true;
		}
		else if (havePoint && !haveNumberAfterPoint)
		{
			abort(STOP_INVALID_NUMBER, "A digit must follow a decimal point.");
		}
		else
			break;

		if (m_stop != STOP_NONE)
			break;
	} while (1);
}

void CLexer::enumerate(FILE *out)
{
	fprintf(out, "[\n");
	for (int i = 0; i < m_numTokens; i++)
	{
		const Token &token = m_tokens[i];

		if (i != 0)
			fprintf(out, "\n");

		char text[OUTPUT_TEXT_SIZE];
		switch (token.type)
		{
			case TOKEN_EOF:
				snprintf(text, OUTPUT_TEXT_SIZE, "%s", "<eof>");
				break;
			case TOKEN_NEWLINE:
				snprintf(text, OUTPUT_TEXT_SIZE, "%s", "<newline>");
				break;
			default:
				snprintf(text, OUTPUT_TEXT_SIZE, "%s", token.text.c_str());
				break;
		}

		fprintf(out, "\t{\n\t\t\"type\": \"%s\",\n\t\t\"id\": %d,\n\t\t\"text\": \"%s\"\n\t}", token_type(token).c_str(), token.type, text);

		if (i != m_numTokens-1)
			fprintf(out, ",");
	}
	fprintf(out, "\n]\n");
}

static constexpr int kTokenChunkSize = 32768;

std::vector<Token> CLexer::run(const std::string &input)
{
	m_numTokenChunks = 1;
	m_maxTokens = kTokenChunkSize * m_numTokenChunks;
	m_tokens.clear();
	m_tokens.resize(m_maxTokens);
	m_stop = STOP_NONE;
	m_input = input;

	while (true)
	{
		Token &token = m_tokens[m_numTokens];
		token = get_token();

		// Overwrite this non-token next time around
		if (token.type != TOKEN_NONE)
			m_numTokens++;
		
		if (token.type == TOKEN_EOF)
			break;

		// Grow the token array
		if (m_numTokens >= m_maxTokens)
		{
			m_numTokenChunks++;
			m_tokens.resize(kTokenChunkSize * m_numTokenChunks);
			m_maxTokens += kTokenChunkSize;
		}
	}

	return m_tokens;
}

Token CLexer::get_token()
{
	Token token;

	int startPos = m_pos;
	m_c = m_input[m_pos];

	if (m_c == LEXER_EOF)
	{
		token.type = TOKEN_EOF;
	}
	else if (isalpha(m_c))
	{
		do advance(); while (isalpha(m_c));

		const int size = m_pos - startPos;
		const auto text = m_input.substr(startPos, size);
		if (is_keyword(text))
		{
			if (text == "PRINT")
				token.type = TOKEN_PRINT;
			else if (text == "WHILE")
				token.type = TOKEN_WHILE;
			else if (text == "ENDWHILE")
				token.type = TOKEN_ENDWHILE;
			else if (text == "LET")
				token.type = TOKEN_LET;
			else if (text == "INPUT")
				token.type = TOKEN_INPUT;
			else if (text == "REPEAT")
				token.type = TOKEN_REPEAT;
		}
		else
			token.type = TOKEN_IDENTIFIER;
	}
	else if (m_c == '"')
	{
		get_string(token);
	}
	else if (isspace(m_c))
	{
		if (m_c == '\n')
		{
			token.type = TOKEN_NEWLINE;
			advance();
		}
		else
		{
			while (isspace(m_c) && m_c != '\n')
				advance();
		}
	}
	else if (m_c == '=')
	{
		if (peek() == '=')
		{
			token.type = TOKEN_EQUAL;
			advance();
		}
		else
			token.type = TOKEN_ASSIGN;
		advance();
	}
	else if (m_c == '>')
	{
		if (peek() == '=')
		{
			token.type = TOKEN_GE;
			advance();
		}
		else
			token.type = TOKEN_GREATER;
		advance();
	}
	else if (m_c == '<')
	{
		if (peek() == '=')
		{
			token.type = TOKEN_LE;
			advance();
		}
		else
			token.type = TOKEN_LESS;
		advance();
	}
	else if (m_c == '+')
	{
		token.type = TOKEN_ADD;
		advance();
	}
	else if (m_c == '-')
	{
		token.type = TOKEN_SUBTRACT;
		advance();
	}
	else if (isdigit(m_c))
	{
		get_number(token);
	}
	else if (m_c == '#')
	{
		do advance(); while (m_c != '\n');
	}
	else
	{
		token.type = TOKEN_UNKNOWN;
		advance();
	}

	if (token.type == TOKEN_NONE)
		return token;

	int textStart = 0;
	int textEnd = 0;

	if (token.type == TOKEN_STRING)
	{
		textStart = startPos + 1;
		textEnd = m_pos - 1;
	}
	else
	{
		textStart = startPos;
		textEnd = m_pos;
	}

	int textSize = textEnd - textStart;
	token.text = m_input.substr(textStart, textSize);

	return token;
}

void CLexer::advance()
{
	m_pos++;
	m_column++;

	if (m_pos >= m_input.size())
		m_c = LEXER_EOF;
	else
		m_c = m_input[m_pos];

	if (m_c == '\n')
	{
		m_line++;
		m_column = 1;
	}
}

char CLexer::peek()
{
	if (m_pos+1 >= m_input.size())
	{
		abort(STOP_PEEK_EOF, NULL);
		return LEXER_EOF;
	}
	return m_input[m_pos+1];
}
