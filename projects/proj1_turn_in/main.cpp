#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Token.h"
#include "Scanner.h"

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
  if (argc < 2 )
   {
    cerr << "no file provided" << endl;
    return 1; 
   }

string FileContent = FiletoString(argv[1]); 

  Scanner scanner(FileContent);
    vector<Token> tokens;
    Token token(TokenType::UNDEFINED,"",0);

do {
    token = scanner.scanTokens();
    tokens.push_back(token);
    cout << token.toString() << endl;
  } while (token.getType() != TokenType::END);

  cout << "Total Tokens = " << tokens.size() << endl;
  return 0;
}
