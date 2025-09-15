#pragma once
#include <iostream>
#include <vector>
#include "Predicate.h"
#include "Rule.h"
#include <set>
using namespace std;


class DatalogProgram
{

public:
   DatalogProgram(){}

   void addScheme(Predicate scheme)
   {
      this->schemes.push_back(scheme);
   }

   void addFact(Predicate fact)
   {
      this->facts.push_back(fact);
      for (Parameter& param : fact.getParameters())
      {
         if (!param.getIsID())
         {
            domain.insert(param.getValue());
         }
      }
   }

   void addRule(Rule rule)
   {
      this->rules.push_back(rule);
   }

   void addQuery(Predicate query)
   {
      this->queries.push_back(query);
   }

   string toString()
   {
      stringstream ss;
      ss << "Schemes(" << schemes.size() <<"):\n";
      for (Predicate& scheme : schemes)
      {
         ss << " " << scheme.toString() << "\n";
      }
      ss << "Facts(" << facts.size() <<"):\n";
      for (Predicate& facts : facts)
      {
         ss << " " << facts.toString() << ".\n";
      }
      ss << "Rules(" << rules.size() <<"):\n";
      for (Rule& rules : rules)
      {
         ss << " " << rules.toString() << ".\n";
      }
      ss << "Queries(" << queries.size() <<"):\n";
      for( auto& queries : queries) 
         {
            ss << " " << queries.toString() << "?\n";
         }
      ss << "Domain(" << domain.size() <<"):\n";
      for ( const string& str : domain)
      {
         ss << " " << str << "\n";
      }
      return ss.str();
   }

   const vector<Predicate>& getSchemes() const {
        return schemes;
    }

    const vector<Predicate>& getFacts() const {
        return facts;
    }

    const vector<Rule>& getRules() const {
        return rules;
    }

    const vector<Predicate>& getQueries() const {
        return queries;
    }

private:
   vector<Predicate> schemes;
   vector<Predicate> facts;
   vector<Rule> rules;
   vector<Predicate> queries;
   set<string> domain;

};