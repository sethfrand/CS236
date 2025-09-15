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
Parameter(string value)
{
    this->value = value;
    if (!value.empty() && value.at(0) == '\''){
        this ->isID = false;
    }else{
        this->isID = true; 
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