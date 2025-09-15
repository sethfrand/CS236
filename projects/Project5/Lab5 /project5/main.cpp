#include "Scanner.h"
#include "Parser.h"
#include "Relation.h"
#include "Database.h"
#include "Interpreter.h"
#include "Node.h"
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
    // Check if a file path was provided
    if (argc < 2) {
        cerr << "no file provided" << endl;
        return 1;
    }

    // Read file content
    string input = FiletoString(argv[1]);
    if (input.empty()) {
        return 1; 
    }

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
        Interpreter interpreter(datalogProgram);
    // interpreter.evaluateSchemes();
    // interpreter.evaluateFacts();

    // interpreter.evaluateRules();
    // interpreter.evaluateQueries();
    interpreter.run();

    return 0;
}