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
   Database(){clear();} 

    void clear() {
        relations.clear();
    }
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
    auto it = relations.find(name);
    if (it == relations.end()) {
        throw runtime_error("Relation '" + name + "' not found in the database.");
    }
    return it->second;
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