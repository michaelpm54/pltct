/* SPDX-License-Identifier: GPLv3-or-later */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "token.h"

std::string get_file(const std::string &path);

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		fprintf(stderr, "filename required\n");
		return EXIT_FAILURE;
	}

	/* Lexer */
	CLexer lexer;
	std::vector<Token> tokens;

	try {
		tokens = lexer.run(get_file(argv[1]));
	} catch (const std::runtime_error &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cout << "Something went wrong" << std::endl;
		return EXIT_FAILURE;
	}

	lexer.enumerate(stdout);

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
