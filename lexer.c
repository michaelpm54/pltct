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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>

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

std::string token_type(Token *t)
{
	switch (t->type)
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
void lexer_abort(Lexer *l, StopReason stop, const char * const fmt, Args ... args);

bool is_keyword(const char * const text, int len)
{
	for (int i = 0; i < COUNT_OF(kKeywords); i++)
	{
		if (!strncmp(text, kKeywords[i], len))
		{
			return true;
		}
	}
	return false;
}

char lexer_peek(Lexer *l)
{
	if (l->pos+1 >= l->size)
	{
		lexer_abort(l, STOP_PEEK_EOF, NULL);
		return LEXER_EOF;
	}
	return l->buf[l->pos+1];
}

std::string get_file(const std::string &path)
{
	std::uintmax_t size = 0;
	std::error_code ec{};
	try {
		size = std::filesystem::file_size(std::filesystem::canonical(path), ec);
	} catch (const std::filesystem::filesystem_error& e) {
		throw std::runtime_error("Failed to get input file size: " + std::string(e.what()));
	}
	if (ec != std::error_code{})
		throw std::runtime_error("Failed to get input file size: " + ec.message());

	if (size > (3 * 1024*1024)) // 3 MB
		throw std::runtime_error("Input file too large (> 3 MB)");
	else if (size == 0)
		throw std::runtime_error("Input file empty");

	std::ifstream stream(path, std::ios::in);
	std::string contents{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
	if (contents.empty())
		throw std::runtime_error("Input file empty");
	return contents;
}

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

template<typename ... Args>
void lexer_abort(Lexer *l, StopReason stop, const char * const format, Args ... args)
{
	throw std::runtime_error(string_format("%s: line %d column %d: '%s'",
		get_stop_reason(stop).c_str(), l->line, l->column, string_format(format, args ...).c_str()));
}

void lexer_advance(Lexer *l)
{
	l->pos++;
	l->column++;

	if (l->pos >= l->size)
		l->c = LEXER_EOF;
	else
		l->c = l->buf[l->pos];

	if (l->c == '\n')
	{
		l->line++;
		l->column = 1;
	}
}

void lexer_init(Lexer *l, const char * const filename)
{
	l->stop = STOP_NONE;
	l->c = LEXER_EOF;
	l->pos = 0;
	l->buf = get_file(filename);
	l->size = l->buf.size();
	l->line = 1;
	l->column = 1;
	l->numTokens = 0;
	l->maxTokens = TOKEN_CHUNK_ALLOC_SIZE;
}

void lexer_free(Lexer *l)
{
	free(l->tokens);
}

void lexer_get_string(Lexer *l)
{
	lexer_advance(l);
	while (l->c != '"' && l->c != LEXER_EOF)
	{
		if (l->c == '\\' || l->c == '%' || l->c == '\r' || l->c == '\n' || l->c == '\t')
			lexer_abort(l, STOP_INVALID_STRING, "%c", l->c);
		lexer_advance(l);
	}
	lexer_advance(l);
}

void lexer_skip_whitespace(Lexer *l)
{
	while (l->pos < l->size && isspace(l->c) && l->c != '\n')
		lexer_advance(l);
}

void lexer_get_number(Lexer *l)
{
	bool havePoint = false;
	bool haveNumberAfterPoint = false;
	do {
		lexer_advance(l);

		if (isdigit(l->c))
		{
			if (havePoint && !haveNumberAfterPoint)
				haveNumberAfterPoint = true;
		}
		else if (l->c == '.')
		{
			if (havePoint)
			{
				lexer_abort(l, STOP_INVALID_NUMBER, "Multiple decimal points.");
			}
			else
				havePoint = true;
		}
		else if (havePoint && !haveNumberAfterPoint)
		{
			lexer_abort(l, STOP_INVALID_NUMBER, "A digit must follow a decimal point.");
		}
		else
			break;

		if (l->stop != STOP_NONE)
			break;
	} while (1);
}

void lexer_skip_to_newline(Lexer *l)
{
	while (l->c != '\n')
		lexer_advance(l);
}

void lexer_get_token(Lexer *l, Token *t)
{
	t->type = TOKEN_NONE;
	t->text = NULL;

	int startPos = l->pos;
	l->c = l->buf[l->pos];

	if (l->c == LEXER_EOF)
	{
		t->type = TOKEN_EOF;
	}
	else if (isalpha(l->c))
	{
		while (isalpha(l->c))
		{
			lexer_advance(l);
		}
		const int size = l->pos - startPos;
		const char * const text = &l->buf[startPos];
		if (is_keyword(text, size))
		{
			if (!strncmp(text, "PRINT", size))
				t->type = TOKEN_PRINT;
			else if (!strncmp(text, "WHILE", size))
				t->type = TOKEN_WHILE;
			else if (!strncmp(text, "ENDWHILE", size))
				t->type = TOKEN_ENDWHILE;
			else if (!strncmp(text, "LET", size))
				t->type = TOKEN_LET;
			else if (!strncmp(text, "INPUT", size))
				t->type = TOKEN_INPUT;
			else if (!strncmp(text, "REPEAT", size))
				t->type = TOKEN_REPEAT;
		}
		else
			t->type = TOKEN_IDENTIFIER;
	}
	else if (l->c == '"')
	{
		t->type = TOKEN_STRING;
		lexer_get_string(l);
	}
	else if (isspace(l->c))
	{
		if (l->c == '\n')
		{
			t->type = TOKEN_NEWLINE;
			lexer_advance(l);
		}
		else
			lexer_skip_whitespace(l);
	}
	else if (l->c == '=')
	{
		if (lexer_peek(l) == '=')
		{
			t->type = TOKEN_EQUAL;
			lexer_advance(l);
		}
		else
			t->type = TOKEN_ASSIGN;
		lexer_advance(l);
	}
	else if (l->c == '>')
	{
		if (lexer_peek(l) == '=')
		{
			t->type = TOKEN_GE;
			lexer_advance(l);
		}
		else
			t->type = TOKEN_GREATER;
		lexer_advance(l);
	}
	else if (l->c == '<')
	{
		if (lexer_peek(l) == '=')
		{
			t->type = TOKEN_LE;
			lexer_advance(l);
		}
		else
			t->type = TOKEN_LESS;
		lexer_advance(l);
	}
	else if (l->c == '+')
	{
		t->type = TOKEN_ADD;
		lexer_advance(l);
	}
	else if (l->c == '-')
	{
		t->type = TOKEN_SUBTRACT;
		lexer_advance(l);
	}
	else if (isdigit(l->c))
	{
		t->type = TOKEN_NUMBER;
		lexer_get_number(l);
	}
	else if (l->c == '#')
	{
		lexer_skip_to_newline(l);
	}
	else
	{
		t->type = TOKEN_UNKNOWN;
		lexer_advance(l);
	}

	if (t->type == TOKEN_NONE)
		return;

	int textStart = 0;
	int textEnd = 0;

	if (t->type == TOKEN_STRING)
	{
		textStart = startPos + 1;
		textEnd = l->pos - 1;
	}
	else
	{
		textStart = startPos;
		textEnd = l->pos;
	}

	int textSize = textEnd - textStart;
	t->text = static_cast<char*>(malloc(textSize+1));
	if (t->text)
	{
		memcpy(t->text, &l->buf[textStart], textSize);
		t->text[textSize] = '\0';
	}

	if (t->type == TOKEN_UNKNOWN)
	{
		lexer_abort(l, STOP_UNKNOWN_TOKEN, "%c", l->buf[l->pos]);
	}
}

void lexer_enumerate(Lexer *l, FILE *out)
{
	fprintf(out, "[\n");
	for (int i = 0; i < l->numTokens; i++)
	{
		Token *t = &l->tokens[i];

		if (i != 0)
			fprintf(out, "\n");

		char text[OUTPUT_TEXT_SIZE];
		switch (t->type)
		{
			case TOKEN_EOF:
				snprintf(text, OUTPUT_TEXT_SIZE, "%s", "<eof>");
				break;
			case TOKEN_NEWLINE:
				snprintf(text, OUTPUT_TEXT_SIZE, "%s", "<newline>");
				break;
			default:
				snprintf(text, OUTPUT_TEXT_SIZE, "%s", t->text);
				break;
		}

		fprintf(out, "\t{\n\t\t\"type\": \"%s\",\n\t\t\"id\": %d,\n\t\t\"text\": \"%s\"\n\t}", token_type(t).c_str(), t->type, text);

		if (i != l->numTokens-1)
			fprintf(out, ",");
	}
	fprintf(out, "\n]\n");
}

void lexer_run(Lexer *l)
{
	l->tokens = static_cast<Token*>(malloc(sizeof(Token) * l->maxTokens));

	if (l->tokens == NULL)
		lexer_abort(l, STOP_BAD_FILE, "Failed to allocate token array\n");

	while (l->stop == STOP_NONE)
	{
		lexer_get_token(l, &l->tokens[l->numTokens]);

		if (l->tokens[l->numTokens].type == TOKEN_EOF)
		{
			l->numTokens++;
			break;
		}
		
		// Overwrite this non-token next time around
		if (l->tokens[l->numTokens].type != TOKEN_NONE)
			l->numTokens++;

		// Grow the token array
		if (l->numTokens == l->maxTokens)
		{
			l->tokens = static_cast<Token*>(realloc(l->tokens, TOKEN_CHUNK_ALLOC_SIZE * sizeof(Token) * ((l->numTokens / TOKEN_CHUNK_ALLOC_SIZE) + 1)));
			l->maxTokens += TOKEN_CHUNK_ALLOC_SIZE;
		}
	}
}
