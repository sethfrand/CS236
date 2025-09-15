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
R"(



# project without re-order, no joins


Schemes:

  a(x,y)
  b(x,y,z)

  c(x)
  d(x)
  e(x)

  f(x,y)
  g(x,y)
  h(x,y)

Facts:

  a('1','4').
  a('2','4').
  a('2','5').

  b('5','4','7').
  b('5','4','8').
  b('6','4','8').

Rules:

  c(x) :- a(x,y).
  d(y) :- b(x,y,z).
  e(z) :- b(x,y,z).

  f(x,y) :- b(x,y,z).
  g(x,z) :- b(x,y,z).
  h(y,z) :- b(x,y,z).

Queries:

  a(x,y)?
  b(x,y,z)?

  c(x)?
  d(x)?
  e(x)?

  f(x,y)?
  g(x,y)?
  h(x,y)?




)";

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