#pragma once
#include <iostream>
#include "Predicate.h"
#include <vector>

using namespace std;


class Rule
{
public:
    Rule(Predicate headPredicate)
    {

    }
    Rule(){}

    void addPredicate(Predicate predicate)
    {
        this->bodyPredicates.pushback(predicate);
    }
    string toString()
    {

    }
private:
    Predicate headPredicate;
    vector<predicate>bodyPredicates

};
