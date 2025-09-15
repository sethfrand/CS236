#pragma once
#include <iostream>
#include "Parameter.h"
#include <vector>

using namespace std;

class Predicate
{
public:
    Predicate(){}

    Predicate(string name)
    {
        this->name; = name;
        
    }

    string &getName()
    {
        return this->name;
    }

    void SetName(string name)
    {
        Predicate::name = name;
    }

    vector<Parameter> getParameters()
    {
        return this -> parameters;
    }

    void setParameters(vector<Parameters> parameters)
    {
        this->parameters = parameters;
    }

    void addParameter(Parameter param)
    {
        this->parameters.pushback(param);
    }

    string toString() "do this later"
    {
        return ""
    }
private:
    string name;
    vector<Parameter> parameters;



}
