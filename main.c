/* SPDX-License-Identifier: GPLv3-or-later */

#include <iostream>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "token.h"

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		fprintf(stderr, "filename required\n");
		return EXIT_FAILURE;
	}

	/* Lexer */
	Lexer l;

	try {
		lexer_init(&l, argv[1]);
	} catch (const std::runtime_error &e) {
		std::cout << e.what() << std::endl;
		lexer_free(&l);
		return EXIT_FAILURE;
	}
	catch (...) {
		lexer_free(&l);
		std::cout << "Something went wrong" << std::endl;
		return EXIT_FAILURE;
	}

	try {
		lexer_run(&l);
	} catch (const std::runtime_error &e) {
		std::cout << e.what() << std::endl;
		lexer_free(&l);
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cout << "Something went wrong" << std::endl;
		lexer_free(&l);
		return EXIT_FAILURE;
	}
	lexer_enumerate(&l, stdout);
	lexer_free(&l);

	/* Parser
	Parser p;

	if (EXIT_FAILURE == parser_init(&p, l.tokens, l.numTokens))
	{
		lexer_free(&l);
		parser_free(&p);
		return EXIT_FAILURE;
	}
	lexer_free(&l);

	if (EXIT_FAILURE == parser_run(&p))
	{
		parser_free(&p);
		return EXIT_FAILURE;
	}

	parser_enumerate(&p, stdout);
	parser_free(&p);
	*/

	return EXIT_SUCCESS;
}
