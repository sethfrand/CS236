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
   bool hasRelation(const string& name)
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

    const Relation& getRelation(const string& name) const {
        auto it = relations.find(name);
        if (it == relations.end()) throw runtime_error("Relation not found");
        return it->second;
    }





   private:
    map<string,Relation>relations;

};