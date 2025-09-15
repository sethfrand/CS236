#pragma once
#include <iostream>
#include <vector>
#include "Predicate.h"
#include "Rule.h"
#include <set>
using namespace std;


class DataProgram
{

public:
   int DatalogProgram(){}

   void addScheme(Predicate scheme)
   {
      this->schemes.pushback
   }


private:
   vector<Predicate> schemes;
   vector<Predicate> facts;
   vector<Predicate> rules;
   vector<Predicate> queries;
   set<string> domain;

};
