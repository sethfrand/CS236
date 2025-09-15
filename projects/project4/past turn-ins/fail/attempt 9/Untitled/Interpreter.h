#pragma once
#include "DatalogProgram.h"
#include "Database.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

class Interpreter {
public:
    Interpreter(const DatalogProgram& datalogProgram, Database& database)
        : datalogprogram(datalogProgram), database(database) {}

    void evalSchemes() {
        const vector<Predicate>& schemes = datalogprogram.getSchemes();
        for (const Predicate& schemePredicate : schemes) {
            string relationName = schemePredicate.getName();
            Scheme scheme;
            for (const Parameter& param : schemePredicate.getParameters()) {
                scheme.push_back(param.getValue());
            }
            database.createRelation(relationName, scheme);
        }
    }

    void evalFacts() {
        const vector<Predicate>& facts = datalogprogram.getFacts();
        for (const Predicate& factPredicate : facts) {
            string relationName = factPredicate.getName();
            if (!database.hasRelation(relationName)) continue;

            vector<string> values;
            for (const Parameter& param : factPredicate.getParameters()) {
                values.push_back(param.getValue());
            }

            Relation& relation = database.getRelation(relationName);
            Tuple tuple(values);
            relation.addTuple(tuple);
        }
    }

    string evalRules() {
        stringstream ss;
        const vector<Rule>& rules = datalogprogram.getRules();

        ss << "Rule Evaluation" << endl;
        
        if (rules.empty()) {
            ss << "\nSchemes populated after 1 passes through the Rules." << endl;
            return ss.str();
        }

        int numPasses = 0;
        bool tuplesAdded;

        do {
            tuplesAdded = false;
            numPasses++;
            cout << "Pass " << numPasses << " through rules" << endl;

            for (const Rule& rule : rules) {
                int tuplesBefore = countTotalTuples();
                evalRule(rule);
                int tuplesAfter = countTotalTuples();

                if (tuplesAfter > tuplesBefore) tuplesAdded = true;
                ss << rule.toString() << "\n";
            }
        } while (tuplesAdded);

        ss << "\nSchemes populated after " << numPasses << " passes through the Rules." << endl;
        return ss.str();
    }

    void evalRule(const Rule& rule) {
    string headName = rule.getHeadPredicate().getName();
    
    // Create a single relation from all body predicates
    bool firstPredicate = true;
    Relation joinedRelation;
    
    // Keep track of all variables used in the rule body
    map<string, vector<pair<string, int>>> variableLocations; // variable -> (relName, position)
    
    // Process each body predicate
    for (const Predicate& bodyPred : rule.getBodyPredicates()) {
        string relName = bodyPred.getName();
        
        if (!database.hasRelation(relName)) {
            cout << "Error: Relation " << relName << " does not exist" << endl;
            return;
        }
        
        // Get the original relation and its scheme
        Relation relation = database.getRelation(relName);
        
        // Create a new relation with variable names matching the rule
        Relation renamedRelation = relation;
        
        // Apply selections and get variable information
        for (size_t i = 0; i < bodyPred.getParameters().size(); i++) {
            const Parameter& param = bodyPred.getParameters()[i];
            
            if (!param.getIsID()) {
                // Constant value - apply selection
                renamedRelation = renamedRelation.select(i, param.getValue());
            } else {
                // Variable - record its location
                string varName = param.getValue();
                variableLocations[varName].push_back(make_pair(relName, i));
            }
        }
        
        // Join with the current result
        if (firstPredicate) {
            joinedRelation = renamedRelation;
            firstPredicate = false;
        } else {
            // Need to implement proper join logic here
            // For now, assuming simple case works
            joinedRelation = joinedRelation.join(renamedRelation);
        }
    }
    
    // Get head predicate variables
    vector<string> headVars;
    for (const Parameter& param : rule.getHeadPredicate().getParameters()) {
        if (param.getIsID()) {
            headVars.push_back(param.getValue());
        }
    }
    
    // Create a new relation with only the columns needed for the head predicate
    Scheme headScheme;
    for (const string& var : headVars) {
        headScheme.push_back(var);
    }
    
    // Create empty relation with head scheme
    Relation headRelation(headName, headScheme);
    
    // Add tuples from the joined relation, projecting only the needed columns
    for (const Tuple& tuple : joinedRelation.getTuples()) {
        vector<string> newTupleValues;
        
        for (const string& headVar : headVars) {
            // Find this variable in the joined relation
            int foundIndex = -1;
            for (size_t i = 0; i < joinedRelation.getScheme().size(); i++) {
                if (joinedRelation.getScheme()[i] == headVar) {
                    foundIndex = i;
                    break;
                }
            }
            
            if (foundIndex >= 0) {
                newTupleValues.push_back(tuple[foundIndex]);
            } else {
                cout << "Error: Head variable " << headVar << " not found in joined relation" << endl;
                return;
            }
        }
        
        headRelation.addTuple(Tuple(newTupleValues));
    }
    
    // Add all tuples from headRelation to the database relation
    Relation& dbRelation = database.getRelation(headName);
    int beforeCount = dbRelation.getTuples().size();
    
    for (const Tuple& tuple : headRelation.getTuples()) {
        dbRelation.addTuple(tuple);
    }
    
    int addedCount = dbRelation.getTuples().size() - beforeCount;
    cout << "Added " << addedCount << " tuples to relation " << headName << endl;
}
    Relation applySelectionsAndProjections(Relation relation, const Predicate& predicate) {
    try {
        // Skip all constant parameters and only use variable parameters
        vector<size_t> projectIndices;
        vector<string> variableNames;
        map<string, size_t> seenVariables;
        
        // Handle constants and track variables
        for (size_t i = 0; i < predicate.getParameters().size(); i++) {
            const Parameter& param = predicate.getParameters()[i];
            
            if (!param.getIsID()) {
                // Constant - apply selection
                relation = relation.select(i, param.getValue());
            } else {
                // Variable - check if seen before
                string varName = param.getValue();
                if (seenVariables.count(varName) > 0) {
                    // Seen before - apply equality selection
                    relation = relation.select(seenVariables[varName], i);
                } else {
                    // First time seeing this variable
                    seenVariables[varName] = i;
                    projectIndices.push_back(i);
                    variableNames.push_back(varName);
                }
            }
        }
        
        // Apply projection and renaming
        if (!variableNames.empty()) {
            relation = relation.project(projectIndices);
            relation = relation.rename(Scheme(variableNames));
        }
        
        return relation;
    } catch (const std::exception& e) {
        cout << "Error: " << e.what() << endl;
        return relation;
    }
}

    int countTotalTuples() {
        int total = 0;
        for (const auto& relationName : database.getRelationNames()) {
            total += database.getRelation(relationName).getTuples().size();
        }
        return total;
    }

    string evalQueries() {
        stringstream ss;
        ss << "Query Evaluation\n";
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
                ss << " Error processing query: " << e.what() << endl;
            }
        }
        
        return ss.str();
    }

private:
    DatalogProgram datalogprogram;
    Database database;
};