/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#ifndef __LEXER__H__
#define __LEXER__H__

#include <vector>
#include <string>

#include "inputbuf.h"

// ------- token types -------------------

typedef enum { END_OF_FILE = 0,
    REAL, INT, BOOLEAN, STRING,
    WHILE,TRUE, FALSE, COMMA, COLON, SEMICOLON,
    LBRACE, RBRACE, LPAREN, RPAREN,
    EQUAL, PLUS, MINUS, MULT, DIV, AND, OR, XOR, NOT,
    GREATER, GTEQ, LESS, LTEQ, NOTEQUAL,
    ID, NUM, REALNUM, STRING_CONSTANT, ERROR
} TokenType;

class Token {
  public:
    void Print();

    std::string lexeme;
    TokenType token_type;
    int line_no;
};

class LexicalAnalyzer {
  public:
    Token GetToken();
    TokenType UngetToken(Token);
    LexicalAnalyzer();
    int get_line_no();

  private:
    std::vector<Token> tokens;
    int line_no;
    Token tmp;
    InputBuffer input;

    bool SkipSpace();
    bool IsKeyword(std::string);
    TokenType FindKeywordIndex(std::string);
    Token ScanIdOrKeyword();
    Token ScanStringCons();
    Token ScanNumber();
};

#endif  //__LEXER__H__
