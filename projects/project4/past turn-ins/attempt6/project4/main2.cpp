#include "Scanner.h"
#include "Parser.h"
#include "Relation.h"
#include "Database.h"
#include "Interpreter.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string FiletoString(const string& path)
{
    ifstream input_file(path);
    if (!input_file.is_open()) {
        cerr << "Error opening file: " << path << endl;
        return "";
    }
    return string((istreambuf_iterator<char>(input_file)), istreambuf_iterator<char>());
}

int main(int argc, char* argv[])
{
    // Hard-coded input for testing
    string input = 
R"(Schemes:
  snap(S,N,A,P)
  csg(C,S,G)
  cn(C,N)
  ncg(N,C,G)

Facts:
  snap('12345','C. Brown','12 Apple St.','555-1234').
  snap('22222','P. Patty','56 Grape Blvd','555-9999').
  snap('33333','Snoopy','12 Apple St.','555-1234').
  csg('CS101','12345','A').
  csg('CS101','22222','B').
  csg('CS101','33333','C').
  csg('EE200','12345','B+').
  csg('EE200','22222','B').

Rules:
  cn(c,n) :- snap(S,n,A,P),csg(c,S,G).
  ncg(n,c,g) :- snap(S,n,A,P),csg(c,S,g).

Queries:
  cn('CS101',Name)?
  ncg('Snoopy',Course,Grade)?)";

    // Uncomment this part if you want to go back to file input
    /*
    if (argc < 2) {
        cerr << "no file provided" << endl;
        return 1;
    }
    string input = FiletoString(argv[1]);
    if (input.empty()) {
        return 1; 
    }
    */

    // Scanner and Parser
    Scanner scanner(input);
    vector<Token> tokens;
    Token token = scanner.scanTokens();
    while (token.getType() != TokenType::END) {
        tokens.push_back(token);
        token = scanner.scanTokens();
    }
    tokens.push_back(token); // Add the END token

    Parser parser(tokens);
    parser.parse();

    DatalogProgram datalogProgram = parser.getDatalogProgram();

    // Database and Interpreter
    Database database;
    Interpreter interpreter(datalogProgram, database);
    interpreter.evalSchemes();
    interpreter.evalFacts();

    // Run queries and output the results
    cout << interpreter.evalRules();
    cout << interpreter.evalQueries();

    return 0;
}