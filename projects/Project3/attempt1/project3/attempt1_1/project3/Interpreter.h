#pragma once
#include "DatalogProgram.h"
#include "Database.h"
#include <iostream>

using namespace std; 

class Interpreter
{
public:
    Interpreter(const DatalogProgram& datalogProgram, Database& database) 
        : datalogprogram(datalogProgram), database(database) {}

    void evalSchemes()
    {
        const vector<Predicate>& schemes = datalogprogram.getSchemes();
        for (const Predicate& schemePredicate : schemes)
        {
            string relationName = schemePredicate.getName();
            Scheme scheme;
            for (const Parameter& param : schemePredicate.getParameters())
            {
                scheme.push_back(param.getValue());
            }
            database.createRelation(relationName, scheme);
        }
    }

    void evalFacts()
    {
        const vector<Predicate>& facts = datalogprogram.getFacts();
        for (const Predicate& factPredicate : facts)
        {
            string relationName = factPredicate.getName();
            vector<string> values;
            for (const Parameter& param : factPredicate.getParameters())
            {
                values.push_back(param.getValue());
            }
            Tuple tuple(values);
            database.getRelation(relationName).addTuple(tuple);
        }
    }

    string evalQueries()
    {

        stringstream ss;
        const vector<Predicate>& queries = datalogprogram.getQueries();
        for (const Predicate& queryPredicate : queries)
        {
            ss << queryPredicate.toString() << "?";
            string relationName = queryPredicate.getName();
            Relation relation = database.getRelation(relationName);

            Relation result = relation;
            vector<int> projectColumns;
            Scheme newScheme;

            for (size_t i = 0; i < queryPredicate.getParameters().size(); i++)
            {
                if (!queryPredicate.getParameters()[i].getIsID())
                {
                    result = result.select(i, queryPredicate.getParameters()[i].getValue());
                }
                else
                {
                    projectColumns.push_back(i);
                }
            }

            if (!projectColumns.empty())
            {
                result = Project(result, projectColumns);
            }

            cout << ss.str() << endl;
            cout << result.toString();
        }
        return ss.str();
    }

    Relation Project(const Relation& relation, const vector<int>& columns)
    {
        Scheme newScheme;
        for (int colIndex : columns)
        {
            newScheme.push_back(relation.getScheme()[colIndex]);
        }

        Relation result(relation.getName(), newScheme);
        for (const auto& tuple : relation.getTuples())
        {
            vector<string> newValues;
            for (int colIndex : columns)
            {
                newValues.push_back(tuple.at(colIndex));
            }
            Tuple newTuple(newValues);
            result.addTuple(newTuple);
        }
        return result;
    }

private:
    DatalogProgram datalogprogram;
    Database database;
};
