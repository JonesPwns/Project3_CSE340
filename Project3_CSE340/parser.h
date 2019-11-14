/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

class Parser {
private:
	LexicalAnalyzer lexer;

	Token expect(TokenType expected_type);
	Token peek();


};

#endif

