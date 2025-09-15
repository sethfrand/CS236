#include "Scanner.h"
#include "Parser.h"
#include "Relation.h"
#include "Database.h" // Assuming you have this
#include "Interpreter.h" // Assuming you have this
#include <iostream>
#include <fstream>

using namespace std;

int main() {
    // Input string
    string input =
        "Schemes: SK(A,B)\n"
        "Facts: SK('a','c'). SK('b','c'). SK('b','b'). SK('b','c').\n"
        "Rules:\n"
        "Queries: SK(X,'c')? SK('b','c')? SK(X,X)? SK(X,Y)? SK('c','c')?\n"
        "END";

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
    interpreter.evaluateSchemes();
    interpreter.evaluateFacts();

    // Run the queries and output the results
    cout << interpreter.evaluateQueries() << endl;

    return 0;
}
