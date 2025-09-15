#pragma once
#include <iostream>
#include "Parameter.h"
#include <vector>
#include <sstream>

using namespace std;

class Predicate
{
public:
    Predicate(){}
 /*    string getValue() 
    {
        return this->vaslue;
    } */

    Predicate(string name)
    {
        this->name = name;
        
    }

    string &getName()
    {
        return this->name;
    }

    void setName(string name)
    {
        this->name = name;
    }

    vector<Parameter> getParameters()
    {
        return this -> parameters;
    }

    void setParameters(vector<Parameter> parameters)
    {
        this->parameters = parameters;
    }

    void addParameter(Parameter param)
    {
        this->parameters.push_back(param);
    }

    string toString() 
    {
        stringstream ss;
        ss << name <<  "(";
        for (size_t i = 0; i < parameters.size(); i++)
        {
            ss << parameters[i].getValue();
            if (i < parameters.size() - 1)
            {
                ss << ",";
            }
        }
        ss << ")";
        return ss.str();
    }
private:
    string name;
    vector<Parameter> parameters;
};
