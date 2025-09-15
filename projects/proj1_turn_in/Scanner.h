#pragma once
#include <string>
#include "Token.h"
#include <cctype>
using namespace std;

class Scanner 
{
private:
    string input;
    int line;
    size_t index;

    char currentChar()
    {
        if (index < input.size())
        {
            return input[index];
        }
        return '\0';
    }

    char next()
    {
        if (index < input.size())
        {
            char c = input[index++];
            if (c == '\n')
            {
                line++;
            }
            return c;
        }
        return '\0';
    }

    void whiteSpace()
    {
        while (isspace(currentChar()))
        {
            next();
        }
    }

Token ScanIdent()
    {
	string ident;
	int startline = line;

	while (isalnum(currentChar()) || currentChar() == '_')
	{
	 ident += next();
	 // ident += currentChar(); 
         // next();
	}
	
	if (ident == "Schemes") return Token(TokenType::SCHEMES, ident,startline);
	if (ident == "Facts") return Token(TokenType::FACTS, ident, startline);
	if (ident == "Rules") return Token(TokenType::RULES, ident, startline);
	if (ident == "Queries") return Token(TokenType::QUERIES, ident, startline);
    
	return Token(TokenType::ID, ident, startline);
    }

Token ScanComment()
{
	string value = "#";
	int startline = line; 
	next();
	while (currentChar() != '\n' && currentChar() != '\0')
	{
	  value += next();
	}
	return Token(TokenType::COMMENT,value,startline);


}


Token ScanString()
{
    string value = "'";
    int startline = line;
    next(); 

    while (currentChar() != '\0')
    {
        if (currentChar() == '\'')
        {
              // value +=
	        next();

            if (currentChar() == '\'')
            {
                value+= "'";
		next();
            }
            else
            {
               // value+= "'";
		return Token(TokenType::STRING, value+"'", startline);
            }
        }
        else if (currentChar() == '\n'|| currentChar() == '\r')
        {
            return Token(TokenType::UNDEFINED,value,startline);
        }
        else
        {
            value += next();
        }
    }

    return Token(TokenType::UNDEFINED, value, startline);
}

public:
    Scanner(const string &input) : input(input), line(1), index(0) {}

    Token scanTokens()
    {
     while (index < input.size())
     {
        whiteSpace();
        if (index >= input.size())
        {
            return Token(TokenType::END, "", line);
        }
 
        char c = currentChar();
        switch(c)
        {
            case ',': next(); return Token(TokenType::COMMA, ",", line);
            case '.': next(); return Token(TokenType::PERIOD, ".", line);
            case '?': next(); return Token(TokenType::Q_MARK, "?", line);
            case '(': next(); return Token(TokenType::LEFT_PAREN, "(", line);
            case ')': next(); return Token(TokenType::RIGHT_PAREN, ")", line);
            case ':':
		next();
                if (currentChar() == '-')
                {
                   next(); 
		   return Token(TokenType::COLON_DASH, ":-", line);
                }
                return Token(TokenType::COLON, ":", line);
            case '*': next(); return Token(TokenType::MULTIPLY, "*", line);
            case '+': next(); return Token(TokenType::ADD, "+", line);
            case '\'': return ScanString();
	    case '#' : return ScanComment();
	    default: 
		if (isalpha(c)) return ScanIdent();
		string undefined_char(1,c);
		next();
		return Token(TokenType::UNDEFINED,undefined_char,line);

        }
    }
	return Token(TokenType::END,"",line);
   }

};
