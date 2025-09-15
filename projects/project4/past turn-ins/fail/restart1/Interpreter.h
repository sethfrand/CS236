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
            
            // Check if the relation exists
            if (!database.hasRelation(relationName)) {
                cerr << "Error: Relation '" << relationName << "' not found in the database." << endl;
                continue;
            }
            
            vector<string> values;
            for (const Parameter& param : factPredicate.getParameters()) {
                values.push_back(param.getValue());
            }

            // Check tuple size against scheme size
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
        }
    }

string evalRules() {
    stringstream ss;
    const vector<Rule>& rules = datalogprogram.getRules();
    
    if (rules.empty()) {
        return "Rule Evaluation\n";
    }

    ss << "Rule Evaluation" << endl;
    int numPasses = 0;
    bool tuplesAdded;

    do {
        tuplesAdded = false;
        numPasses++;
        
        for (const Rule& rule : rules) {
            int tupleAddedByRule = evalRule(rule);
            if (tupleAddedByRule > 0) {
                tuplesAdded = true;
            }
            
            ss << rule.toString() << "." << endl;
            
            // Output tuples added by this rule
            if (tupleAddedByRule > 0) {
                for (const Tuple& tuple : lastResultRelation.getTuples()) {
                    ss << "  ";
                    for (size_t i = 0; i < lastResultRelation.getScheme().size(); i++) {
                        ss << lastResultRelation.getScheme()[i] << "='" << tuple[i] << "'";
                        if (i < lastResultRelation.getScheme().size() - 1) {
                            ss << ", ";
                        }
                    }
                    ss << endl;
                }
            }
        }
        
    } while (tuplesAdded);

    ss << endl << "Schemes populated after " << numPasses << " passes through the Rules." << endl;
    ss << "Query Evaluation" << endl;
    
    return ss.str();
}

int evalRule(const Rule& rule) {
    const Predicate& headPredicate = rule.getHeadPredicate();
    string headName = headPredicate.getName();
    
    // Evaluate each predicate on the right-hand side
    vector<Relation> intermediateResults;
    const vector<Predicate>& bodyPredicates = rule.getBodyPredicates();
    
    for (const Predicate& bodyPredicate : bodyPredicates) {
        string relationName = bodyPredicate.getName();
        if (!database.hasRelation(relationName)) {
            continue; // Skip if relation does not exist
        }
        
        Relation result = database.getRelation(relationName);
        
        // Apply select, project, and rename operations based on the predicate
        map<string, size_t> variablePositions;
        vector<size_t> projectIndices;
        vector<string> variableNames;
        
        const auto& parameters = bodyPredicate.getParameters();
        for (size_t i = 0; i < parameters.size(); i++) {
            const Parameter& param = parameters[i];
            if (!param.getIsID()) {
                result = result.select(i, param.getValue());
            } else {
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
        
        // Ensure projectIndices matches the scheme size
        if (projectIndices.size() != result.getScheme().size()) {
            projectIndices.clear();
            for (size_t i = 0; i < result.getScheme().size(); i++) {
                projectIndices.push_back(i);
            }
        }
        
        Relation projected = result.project(projectIndices);
        Relation renamed = projected.rename(Scheme(variableNames));
        
        intermediateResults.push_back(renamed);
    }
    
    // Join the intermediate results
    Relation joinedResult;
    if (!intermediateResults.empty()) {
        joinedResult = intermediateResults[0];
        for (size_t i = 1; i < intermediateResults.size(); i++) {
            joinedResult = joinedResult.join(intermediateResults[i]);
        }
        
        // Ensure joinedResult scheme matches the expected scheme
        const Scheme& expectedScheme = database.getRelation(headName).getScheme();
        vector<size_t> finalProjectIndices;
        for (const string& attr : expectedScheme) {
            size_t index = 0;
            for (size_t j = 0; j < joinedResult.getScheme().size(); j++) {
                if (joinedResult.getScheme()[j] == attr) {
                    index = j;
                    break;
                }
            }
            finalProjectIndices.push_back(index);
        }
        
        Relation projected = joinedResult.project(finalProjectIndices);
        joinedResult = projected.rename(expectedScheme);
    }
    
    // Add tuples to the target 
    Relation& targetRelation = database.getRelation(headName);
    int tuplesAdded = 0;
    
    for (const Tuple& tuple : joinedResult.getTuples()) {
        size_t beforeSize = targetRelation.getTuples().size();
        targetRelation.addTuple(tuple);
        size_t afterSize = targetRelation.getTuples().size();
        if (afterSize > beforeSize) {
            tuplesAdded++;
        }
    }
    
    // Store result relation variable
    lastResultRelation = Relation(headName, database.getRelation(headName).getScheme());
    for (const Tuple& tuple : joinedResult.getTuples()) {
        size_t beforeSize = targetRelation.getTuples().size();
        targetRelation.addTuple(tuple);
        size_t afterSize = targetRelation.getTuples().size();
        if (afterSize > beforeSize) {
            lastResultRelation.addTuple(tuple);
            tuplesAdded++;
        }
    }
    
    return tuplesAdded;
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
                        ss << " ";
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
    Relation lastResultRelation;
};