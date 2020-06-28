/* SPDX-License-Identifier: GPLv3-or-later */

/* TODO:
	- Assign ascii values to the enum to make comparison faster.
	- Use unicode instead?
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>

#define LEXER_EOF '\0'
#define ERR_MAX_LEN 128

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
} Lexer;

typedef struct Token_s
{
	TokenType type;
	char *text;
} Token;

const char *kKeywords[] =
{
	"WHILE",
	"ENDWHILE",
	"LET",
	"PRINT",
	"INPUT",
	"REPEAT",
};

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

void lexer_abort(Lexer *l, StopReason stop, const char *fmt, ...);

bool is_keyword(const char *text, int len)
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
		lexer_abort(l, STOP_PEEK_EOF, "");
	return l->buf[l->pos+1];
}

bool lexer_good(Lexer *l)
{
	return l->stop == STOP_NONE;
}

char *get_file(const char *path, long unsigned int *size)
{
	FILE *file = fopen(path, "r");
	if (!file)
	{
		fprintf(stderr, "Failed to open file\n");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	printf("File path: %s\n", path);
	printf("File size: %d bytes\n", *size);

	const int MAX_SIZE = 1048576 * 2; // 2 MB

	char *buf = malloc(MAX_SIZE);
	*size = fread(buf, 1, MAX_SIZE, file) + 1;
	fclose(file);

	if (ferror(file))
	{
		fprintf(stderr, "fread error\n");
		clearerr(file);
	}

	if (*size >= MAX_SIZE)
	{
		fprintf(stderr, "File too big (> 2MB)\n");
		fclose(file);
		return NULL;
	}

	buf[*size - 1] = '\0';

	return realloc(buf, *size);
}

void lexer_abort(Lexer *l, StopReason stop, const char *format, ...)
{
	l->stop = stop;

	if (l->errMsg)
		free(l->errMsg);

	if (format && format[0] != '\0')
	{
		l->errMsg = malloc(ERR_MAX_LEN);
	
		va_list args;
    	va_start(args, format);
		vsnprintf(l->errMsg, ERR_MAX_LEN, format, args);
    	va_end(args);	
	}
}

void lexer_advance(Lexer *l)
{
	if (l->pos+1 == l->size || l->buf[l->pos+1] == '\0')
	{
		lexer_abort(l, STOP_EOF, "");
	}

	l->c = l->buf[++l->pos];
	l->column++;
}

void lexer_init(Lexer *l, const char *filename)
{
	l->stop = STOP_NONE;
	l->errMsg = NULL;
	l->c = LEXER_EOF;
	l->pos = 0;
	l->size = 0;
	l->buf = get_file(filename, &l->size);
	l->line = 1;
	l->column = 1;
	if (!l->buf)
	{
		lexer_abort(l, STOP_BAD_FILE, "");
	}
}

void lexer_free(Lexer *l)
{
	free(l->buf);
	if (l->errMsg)
		free(l->errMsg);
}

void lexer_get_string(Lexer *l)
{
	do {
		lexer_advance(l);

		if (l->stop != STOP_NONE)
			break;

		if (l->c == '"')
			break;

		if (l->c == '\\')
			lexer_abort(l, STOP_INVALID_STRING, "\\");

		if (l->c == '%')
			lexer_abort(l, STOP_INVALID_STRING, "%%");
	} while (1);

	lexer_advance(l);
}

void lexer_skip_whitespace(Lexer *l)
{
	while (lexer_good(l) && l->pos < l->size && isspace(l->c) && l->c != '\n')
	{
		lexer_advance(l);
		if (l->c == EOF || l->c == LEXER_EOF)
			lexer_abort(l, STOP_EOF, "");
	}
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
				puts("NANI");
				lexer_abort(l, STOP_INVALID_NUMBER, "Multiple decimal points.");
			}
			else
				havePoint = true;
		}
		else if (havePoint && !haveNumberAfterPoint)
		{
			puts("NANI");
			lexer_abort(l, STOP_INVALID_NUMBER, "A digit must follow a decimal point.");
		}
		else
			break;

		if (l->stop != STOP_NONE)
			break;
	} while (1);
}

void lexer_get_token(Lexer *l, Token *t)
{
	int startPos = l->pos;
	l->c = l->buf[l->pos];

	if (isalpha(l->c))
	{
		while (l->stop == STOP_NONE && isalpha(l->c))
			lexer_advance(l);
		t->type = is_keyword(&l->buf[startPos], l->pos - startPos) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
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
			l->line++;
			l->column = 1;
			lexer_advance(l);
		}
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
	else
	{
		t->type = TOKEN_UNKNOWN;
		lexer_advance(l);
	}

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
	t->text = malloc(textSize+1);
	if (t->text)
	{
		memcpy(t->text, &l->buf[textStart], textSize);
		t->text[textSize] = '\0';
	}

	if (t->type == TOKEN_UNKNOWN)
	{
		lexer_abort(l, STOP_UNKNOWN_TOKEN, "line %d column %d: '%s'", l->line, l->column, t->text);
	}
}

char *token_type(Token *t)
{
	switch (t->type)
	{
		case TOKEN_KEYWORD:
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

char *get_stop_reason(StopReason r)
{
	switch (r)
	{
		case STOP_NONE:
			return "None";
		case STOP_EOF:
			return "EOF";
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

void lexer_print_error(Lexer *l)
{
	const int MAX_LEN = 128;
	char *msg = malloc(MAX_LEN);
	snprintf(msg, MAX_LEN,
		"%s: line %d column %d: '%s'",
		get_stop_reason(l->stop), l->line, l->column,
		l->errMsg ? l->errMsg : "<no msg>"
	);
	fprintf(stderr, "%s\n", msg);
	free(msg);
}

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		fprintf(stderr, "filename required\n");
		return EXIT_FAILURE;
	}

	Lexer l;
	lexer_init(&l, argv[1]);

	puts("\"Lexer\": {");

	int numTokens = 0;

	while (lexer_good(&l))
	{
		Token t;
		t.type = TOKEN_NONE;
		t.text = NULL;

		lexer_get_token(&l, &t);

		if (lexer_good(&l) && t.type != TOKEN_NONE)
		{
			// Prevent writing a comma on the last token because JSON doesn't like it.
			if (numTokens)
				puts(",");
			numTokens++;

			if (t.text)
			{
				if (t.type == TOKEN_NEWLINE)
				{
					printf("\t\"Token\": {\n\t\t\"type\": \"%s\",\n\t\t\"id\": %d,\n\t\t\"text\": \"%s\"\n\t}", token_type(&t), t.type, "<newline>");
				}
				else
				{
					printf("\t\"Token\": {\n\t\t\"type\": \"%s\",\n\t\t\"id\": %d,\n\t\t\"text\": \"%s\"\n\t}", token_type(&t), t.type, t.text);
				}
			}
		}

		if (t.text)
			free(t.text);
	}

	puts("\n}");

	if (l.stop != STOP_NONE && l.stop != STOP_EOF)
	{
		lexer_print_error(&l);
		lexer_free(&l);
		return EXIT_FAILURE;
	}

	lexer_free(&l);
	return EXIT_SUCCESS;
}
