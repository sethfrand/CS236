#pragma once
#include <iostream>
#include <string>
using namespace std;


class Parameter
{
private:
    string value;
    bool isID;

public:
    Parameter(std::string value) {
        if (value.empty()) {
            this->value = "";
            this->isID = false;
        } else {
            this->value = value;
            this->isID = (value[0] != '\'');
        }
    }
string getValue() const
{
    return this->value;
} 
bool getIsID() const
{
    return this -> isID;
}
void setValue(string value)
{
    this->value = value; 
}

void setIsID (bool isID){
    this->isID = isID;
}

bool isConstant () const {
    return !isID;
}

};