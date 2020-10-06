cmake_minimum_required(VERSION 3.15.6)

project(PCC)

set(SOURCES
	src/main.cpp
	src/lexer.cpp
	src/parser.cpp
	src/lexer.h
	src/parser.h
	src/token.h
)

add_executable(pcc ${SOURCES})
set_target_properties(
	pcc
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)
