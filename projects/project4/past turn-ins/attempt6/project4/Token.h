#pragma once 

#include <sstream> 
#include <string>
using namespace std;

enum TokenType
{
	COMMA, //PUNCTUATION 
	PERIOD,
	Q_MARK,
	LEFT_PAREN,
	RIGHT_PAREN,
	COLON, //OPERATIONS
	COLON_DASH,
	MULTIPLY,
	ADD,
	SCHEMES, //KEYWORDS
	FACTS,
	RULES,
	QUERIES,
	ID, //IDENTIFIERS
	STRING,
	COMMENT, //OTHER
	UNDEFINED,
	END
};

class Token
{
	private:
		TokenType type;
		string value;
		int line;

public:
	Token(TokenType type, const string& value, int line) : type(type), value(value), line(line) {}
	
    TokenType getType() const { return type; }
    const string& getValue() const { return value; }
    int getLine() const { return line; }

string toString() 
const{
	stringstream out;
	out << "(" << typeName(type) << "," << "\"" << value << "\"" << "," << line << ")";
	return out.str();
     }

string typeName(TokenType type)
const{
	switch(type)
	{
		case COMMA: return "COMMA";//PUNCTUATIONS
		case PERIOD: return "PERIOD";
		case Q_MARK: return "Q_MARK";
		case LEFT_PAREN: return "LEFT_PAREN";
		case RIGHT_PAREN: return "RIGHT_PAREN";
		case COLON: return "COLON"; //OPERATIONS
		case COLON_DASH: return "COLON_DASH";
		case MULTIPLY: return "MULTIPLY";
		case ADD: return "ADD"; 
		case SCHEMES: return "SCHEMES"; //Keywords
		case FACTS: return "FACTS";
		case RULES: return "RULES";
		case QUERIES: return "QUERIES"; 
		case ID: return "ID"; //ident
		case STRING: return "STRING";
		case COMMENT: return "COMMENT";
		case UNDEFINED: return "UNDEFINED";
		case END: return "END";

	}

	return "";
}

};
