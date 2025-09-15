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
       //std::cout << "error" << std::endl;
	   throw tokens.at(0);
    }

    void match(TokenType t)
	{
	 if (tokenType() == TokenType::COMMENT)
	 {
		advanceToken();
	 }
	 if (tokenType() == t)
	  {
	    advanceToken();
	  }else{
		throwError();	
	
	       }
	}
     
    void idList(Predicate &pred)
    {
	 if (tokenType() == TokenType::COMMA)
	  {
	    match(TokenType::COMMA);
		pred.addParameter(Parameter(this->tokens.at(0).getValue()));
	    match(TokenType::ID);
		//pred.addParameter(Parameter(this->tokens.at(0).getValue()));
	    idList(pred);
	  }
	  
	}

    void scheme()
	{
	  Predicate pred = Predicate();
	  pred.setName(this->tokens.at(0).getValue());
	  match(TokenType::ID);
	  match(TokenType::LEFT_PAREN);

	  pred.addParameter(Parameter(this->tokens.at(0).getValue()));

	  match(TokenType::ID);
	  idList(pred);
	  match(TokenType::RIGHT_PAREN);
	  this->datalog.addScheme(pred);
	  //pred.clear();
	}

	void schemeList()
	{
		if (tokenType() == TokenType::ID)
		{
			scheme();
			schemeList();
		}

	}

	void factList()
	{
		if (tokenType() == TokenType::ID)
		{
			fact();
			factList();
		}
	}

	void ruleList()
	{
		if (tokenType() == TokenType::ID)
		{
			rule();
			ruleList();
		}

	}

	void queryList()
	{
		if (tokenType() == TokenType::ID)
		{
			query();
			queryList();
		}
	}

	void fact()
	{
	  Predicate pred = Predicate();
	  pred.setName(this->tokens.at(0).getValue());
	  match(TokenType::ID);
	  match(TokenType::LEFT_PAREN);

	  pred.addParameter(Parameter(this->tokens.at(0).getValue()));
	  match(TokenType::STRING);
	  stringList(pred);

	  match(TokenType::RIGHT_PAREN);
	  match(TokenType::PERIOD);
	  this->datalog.addFact(pred);
	  //pred.clear();
	}

	void rule()
	{
		Rule rule;
		rule.setHeadPredicate(headPredicate());
		match(TokenType::COLON_DASH);
		rule.addBodyPredicate(predicate());
		predicateList(rule);
		
		if (tokenType() == TokenType::PERIOD)
		{
			match(TokenType::PERIOD);
		}else{
			throwError();
		}

		datalog.addRule(rule);

	}
	
	void query()
	{
		Predicate pred = predicate();
		match(TokenType::Q_MARK);
		datalog.addQuery(pred);
	}


	Predicate headPredicate()
	{
		Predicate pred = Predicate();

		pred.setName(tokens.at(0).getValue());
	  	match(TokenType::ID);
	 	match(TokenType::LEFT_PAREN);

		if (tokenType() == TokenType::ID)
		{
	  		pred.addParameter(Parameter(tokens.at(0).getValue()));
	 		match(TokenType::ID);
		}
		idList(pred);
	  	match(TokenType::RIGHT_PAREN);
	  	return pred;
	}

	Predicate predicate()
	{
		Predicate pred = Predicate();
		pred.setName(tokens.at(0).getValue());
	  	match(TokenType::ID);
	 	match(TokenType::LEFT_PAREN);
		parameter(pred);
		parameterList(pred);
		match(TokenType::RIGHT_PAREN);
		return pred;
	}

	void predicateList(Rule& rule)
	{
		if (tokenType() == TokenType::COMMA)
		{
			match(TokenType::COMMA);
			rule.addBodyPredicate(predicate());
			predicateList(rule);
		}
	}

	void stringList(Predicate& pred)
	{
		if (tokenType() == TokenType::COMMA)
		{
			match(TokenType::COMMA);
			pred.addParameter(Parameter(this->tokens.at(0).getValue()));
			match(TokenType::STRING);
			stringList(pred);

		}
	}

	void parameter(Predicate& pred)
	{
		if (tokenType() == TokenType::STRING)
		{
			pred.addParameter(Parameter(tokens.at(0).getValue()));
			match(TokenType::STRING);
		}else if(tokenType() == TokenType::ID)
			{
			pred.addParameter(Parameter(tokens.at(0).getValue()));
			match(TokenType::ID);
			}
			else{
				throwError();
				}
	}

	void parameterList(Predicate& pred)
	{
		if(tokenType() == TokenType::COMMA)
		{
			match(TokenType::COMMA);
			parameter(pred);
			parameterList(pred);
		}

	}

	void parse()
	{
		try{
			match (TokenType::SCHEMES);
			match(TokenType::COLON);
			scheme();
			schemeList();

			match(TokenType::FACTS);
			match(TokenType::COLON);
			factList();

			match(TokenType::RULES);
			match(TokenType::COLON);
			ruleList();

			match(TokenType::QUERIES);
			match(TokenType::COLON);
			query();
			queryList();


			match(TokenType::END);


			//std::cout << "Success!\n" << std::endl;
			//std::cout << datalog.toString() << std::endl; 
		}

		catch(Token& t)
		{
			//std::cout << "Failure!\n" << t.toString() << std::endl;
			//std::cout << datalog.toString() << std::endl;
			exit(0); 
		}
		
	}
	DatalogProgram getDatalogProgram()
	{
		return datalog;
	}
};

