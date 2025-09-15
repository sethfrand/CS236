#include <iostream>
#include <vector>
#include "Token.h"
#include "Scanner.h"
#include "DatalogProgram.h"
#include "Rule.h"
#include "Predicate.h"
#include "Parameter.h"

class Parser
{
private:
    vector<Token> tokens;
    void parseScheme();
    void parseFact();
    void parseRule();
    void parseQuery();
	DatalogProgram datalog;

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {} 

    TokenType tokenType() const
    {
        return tokens.at(0).getType();
    }

    void advanceToken()
    {
        tokens.erase(tokens.begin());
    }

    void throwError()
    {
       std::cout << "error" << std::endl;
	   throw tokens.at(0);
    }

    void match(TokenType t)
	{
	 if (tokenType() == t)
	  {
	    advanceToken();
	  }else{
		throwError();	
	
	       }
	}
     
    void idList(Predicate &pred)
        {
	 if (tokenType() == COMMA)
	  {
	    match(COMMA);
		pred.addParameter(Parameter(this->tokens.at(0).getValue));
	    match(ID);
		pred.addParameter(Parameter(this->tokens.at(0).getValue));
	    idList();
	  }

	}

    void scheme()
	{
	Predicate pred = Predicate();
	pred.setName(this->tokens.at(0).getValue)
	 match(ID);
	 match(LEFT_PAREN);
	 pred.addParameter(Parameter(this->tokens.at(0).getValue));
	 match(ID);
	 idList(pred);
	 match(RIGHT_PAREN);
	 this->datalog.addSchemes(pred);
	 pred.clear();
	}

	void datalog()
	{
		try{
			match (TokenType:SCHEMES);
			match(Tokentype::COLON);
			scheme();
		}
		catch(Token t)
		{
			cout << "Failure!\n" << endl;
		}


		}
	}
};

