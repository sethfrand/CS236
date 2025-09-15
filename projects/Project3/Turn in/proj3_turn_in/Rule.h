#pragma once
#include <iostream>
#include "Predicate.h"
#include <vector>
#include <sstream>

using namespace std;


class Rule
{
public:
    Rule() {}
    Rule(Predicate headPredicate): headPredicate(headPredicate){}

    void setHeadPredicate(Predicate predicate)
    {
        this->headPredicate = predicate;
    }

    void addBodyPredicate(Predicate predicate)
    {
        this->bodyPredicates.push_back(predicate);
    }
    string toString()
    {
        stringstream ss; 
        ss << headPredicate.toString() << " :- ";
        for (size_t i = 0; i <bodyPredicates.size(); i++)
        {
            ss << bodyPredicates[i].toString();
            if (i < bodyPredicates.size() -1)
            {
                ss << ",";
            }
        } 
        
        return ss.str();
    }
private:
    Predicate headPredicate;
    vector<Predicate> bodyPredicates;
};
