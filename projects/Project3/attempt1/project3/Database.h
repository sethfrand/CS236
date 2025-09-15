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
        relations[relation.getName()] = relation
   }
   bool hasRelation(const string& name)
   {
    return relations.count() > 0;
   }
   void createRelation(const string& name, const Scheme& scheme)
   {
    relations.emplace(name, Relation(name, scheme));
   }





   private:
    map<string,Relation>relations

};