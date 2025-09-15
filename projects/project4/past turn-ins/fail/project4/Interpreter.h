#pragma once
#include "DatalogProgram.h"
#include "Database.h"
#include <iostream>
#include <map>
#include <algorithm>

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

 void evalFacts() {
    const vector<Predicate>& facts = datalogprogram.getFacts();
    
    for (const Predicate& factPredicate : facts) {
        string relationName = factPredicate.getName();
        
        // Debug: Check if the relation exists
        if (!database.hasRelation(relationName)) {
            cerr << "Error: Relation '" << relationName << "' not found in the database." << endl;
            continue;
        }
        
        vector<string> values;
        for (const Parameter& param : factPredicate.getParameters()) {
            values.push_back(param.getValue());
        }

        // Debug: Check tuple size against scheme size
        Relation& relation = database.getRelation(relationName);
        if (values.size() != relation.getScheme().size()) {
            cerr << "Error: Tuple size (" << values.size() 
                 << ") does not match scheme size (" << relation.getScheme().size() 
                 << ") for relation '" << relationName << "'" << endl;
            continue;
        }

        // Create and add the tuple
        Tuple tuple(values);
        relation.addTuple(tuple);

        // Debug: Confirm tuple addition
        //cout << "Added tuple to relation '" << relationName << "': ";
       // for (const string& val : values) {
        //   cout << val << " ";
        //}
       // cout << endl;
    }
}


string evalQueries() {
    stringstream ss;
    const vector<Predicate>& queries = datalogprogram.getQueries();

    for (const Predicate& queryPredicate : queries) {
        try {
            ss << queryPredicate.toString() << "?";

            string relationName = queryPredicate.getName();

            if (!database.hasRelation(relationName)) {
                ss << " No" << endl;
                continue;
            }

            Relation result = database.getRelation(relationName);

            map<string, size_t> variablePositions;
            vector<size_t> projectIndices;
            vector<string> variableNames;
            bool hasVariable = false;

            const auto& parameters = queryPredicate.getParameters();
            for (size_t i = 0; i < parameters.size(); i++) {
                const Parameter& param = parameters[i];

                if (!param.getIsID()) {
                    // Constant parameter
                    result = result.select(i, param.getValue());
                } else {
                    // Variable parameter
                    hasVariable = true;
                    string varName = param.getValue();
                    if (variablePositions.count(varName)) {
                        result = result.select(variablePositions[varName], i);
                    } else {
                        variablePositions[varName] = i;
                        projectIndices.push_back(i);
                        variableNames.push_back(varName);
                    }
                }
            }

            size_t numResults = result.getTuples().size();
            if (numResults == 0) {
                ss << " No" << endl;
            } else {
                ss << " Yes(" << numResults << ")" << endl;

                if (hasVariable) {
                    Relation projected = result.project(projectIndices);
                    Relation renamed = projected.rename(Scheme(variableNames));

                    for (const Tuple& tuple : renamed.getTuples()) {
                        ss << "  ";
                        for (size_t i = 0; i < renamed.getScheme().size(); i++) {
                            ss << renamed.getScheme()[i] << "=" << tuple[i];
                            if (i < renamed.getScheme().size() - 1) {
                                ss << ", ";
                            }
                        }
                        ss << endl;
                    }
                }
            }
        } catch (const std::exception& e) {
            ss << "Error processing query: " << e.what() << endl;
        }
    }

    return ss.str();
}


private:
    DatalogProgram datalogprogram;
    Database database;
};