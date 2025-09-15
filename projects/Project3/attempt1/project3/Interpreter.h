#pragma once
#include "Datalog.h"
#include "Database.h"
#include <iostream>

using namespace std; 

class Interpreter
{
    public:
    Interpreter(const DatalogProgram& datalogProgram, Database& database) : 
    datalogProgram(datalogProgram), database(database) {}

    void evalSchemes()
    {
        vector<Predicate>& schemes = datalogprogram.getSchemes()
        for (const Predicate& schemePredicate : schemes)
        {
            string relationName = schemePredicate.getName();
            Scheme scheme;
            for (const Predicate& param : schemePredicate.getParameters();)
            {
                scheme.pushback(param.GetValue());
            }
        database.createRelation(relationName,scheme);
        }
    }
    
    void evalFacts()
    {
        vector<Predicate>& schemes = datalogprogram.getFacts()
        for (const Predicate& factPredicate : factss)
        {
            string relationName = factPredicate.getName();
            Scheme scheme;
            for (const Predicate& param : factPredicate.getParameters();)
            {
                values.pushback(param.GetValue());
            }
        Tuple tuple(values);
        relation.addtuple(tuple);

        }
    }

    void evalQueries()
    {
        stringstream ss;
        vector<Predicate>& queries = datalogprogram.getQueries()
        for (const Predicate& queryPredicate : queries)
        {
            for (const Predicate& queryPredicate : queries)
            {
                ss << queryPredicate.Tostring() << "?";
                string relationName = queryPredicate.getName();
                Relation relation = database.getRelation(relationName)

                Relation result = relation; 
                vector<int> projectColumns;
                Scheme newScheme; 

                for (size_t = 0; i < queryPredicate.getParameters.size())
                {
                    if (!queryPredicate.getParameters()[i].getIsID)
                    {
                        result = result.select(i, queryPredicate.getParameters()[i].getValue())
                    }
                }
            }

        }

        //build projection and rename operations
    }

    void Project()
    {
        <vector> 
    }



    private:
    DatalogProgram datalogprogram;
    Database database;
}