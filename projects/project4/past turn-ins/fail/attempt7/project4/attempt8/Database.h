#pragma once
#include <vector> 
#include <iostream>
#include "Relation.h"
#include "Tuple.h"
#include "Scheme.h"
#include <map>
#include <string>


using namespace std;

class Database
{
   public:
   Database(){}
   void addRelation(const Relation& relation)
   {
        relations[relation.getName()] = relation;
   }
   bool hasRelation(const string& name) const
   {
    return relations.count(name) > 0;
   }
   void createRelation(const string& name, const Scheme& scheme)
   {
    relations.emplace(name, Relation(name, scheme));
   }

   Relation& getRelation(const string& name)
{
    return relations.at(name);
}

const Relation& getRelation(const string& name) const
{
    return relations.at(name);
}

vector<string> getRelationNames() const
{
    vector<string> names;
    for (const auto&pair : relations)
    {
        names.push_back(pair.first);
    }
    return names;
}



   private:
    map<string,Relation>relations;

};