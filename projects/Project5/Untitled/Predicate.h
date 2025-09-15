#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Parameter.h"

using namespace std;

class Predicate {
public:
    Predicate() {}
    Predicate(string name) : name(name) {}

    const string& getName() const {
        return name;
    }

    void setName(string newName) {
        name = newName;
    }

    vector<Parameter> getParameters() const {
        return parameters;
    }

    vector<string> getParametersAsVector() const {
        vector<string> paramValues;
        for (const auto& param : parameters) {
            paramValues.push_back(param.getValue());
        }
        return paramValues;
    }

    void setParameters(vector<Parameter> newParams) {
        parameters = newParams;
    }

    void addParameter(Parameter param) {
        parameters.push_back(param);
    }

    string toString() const {
        stringstream ss;
        ss << name << "(";
        for (size_t i = 0; i < parameters.size(); i++) {
            ss << parameters[i].getValue();
            if (i < parameters.size() - 1) {
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