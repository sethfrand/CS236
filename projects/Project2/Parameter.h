#pragma once
#include <iostream>

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
    if (value.at(0) == '\''){
        this ->isID = false;
    }else{
        this->isID = true; 
    }

}
string getValue(){
    return this->value;
} 
bool getisID(){
    return this -> value;
}
void setValue() (string value){
    this->value; 
}

void setIsID (bool isID){
    this->isID = isID;
}

}