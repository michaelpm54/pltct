/* SPDX-License-Identifier: GPLv3-or-later */

#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		fprintf(stderr, "filename required\n");
		return EXIT_FAILURE;
	}

	Lexer l;

	if (EXIT_FAILURE == lexer_init(&l, argv[1]))
		return EXIT_FAILURE;

	if (EXIT_FAILURE == lexer_run(&l))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
