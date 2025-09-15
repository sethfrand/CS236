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





   private:
    map<string,Relation>relations;

};