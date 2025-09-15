#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Scheme.h"

using namespace std;

class Tuple: public vector<string>
{
    public:
    Tuple(vector<string> values) : vector<string>(values) { }

    string toString(const Scheme& scheme) const
    {
        const Tuple& tuple = *this;
        stringstream out;
        for (size_t i = 0; i< scheme.size(); i++)
        {
            out << scheme[i] << "=" << tuple[i];
        }
        return out.str();

    }

};