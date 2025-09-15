#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Scheme.h"
#include <string>

using namespace std;

class Tuple: public vector<string>
{
public:
    Tuple(vector<string> values) : vector<string>(values) { }

    string toString(const Scheme& scheme) const
    {
        stringstream out;
        for (size_t i = 0; i < scheme.size(); i++)
        {
            out << scheme[i] << "=" << (*this)[i];
            if (i < scheme.size() - 1)
            {
                out << ", ";
            }
        }
        return out.str();
    }
};