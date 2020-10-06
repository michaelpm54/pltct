cmake_minimum_required(VERSION 3.15.6)

project(pltct)

set(SOURCES
	src/main.cpp
	src/lexer.cpp
	src/parser.cpp
	src/lexer.h
	src/parser.h
	src/token.h
)

add_executable(pltct ${SOURCES})
set_target_properties(
	pltct
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)
