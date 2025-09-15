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
        cout << "Evaluating Rule: " << rule.toString() << endl;
        
        // First, process all body predicates
        vector<Relation> intermediateResults;
        for (const Predicate& bodyPredicate : rule.getBodyPredicates()) {
            if (!database.hasRelation(bodyPredicate.getName())) {
                // If a relation doesn't exist, skip this body predicate
                cout << "Warning: Relation " << bodyPredicate.getName() << " does not exist" << endl;
                continue;
            }
            
            // Get a copy of the relation for this predicate
            Relation baseRelation = database.getRelation(bodyPredicate.getName());
            
            // Apply selections and projections based on the predicate parameters
            Relation result = applySelectionsAndProjections(baseRelation, bodyPredicate);
            
            // Add to our intermediate results
            intermediateResults.push_back(result);
        }

        if (intermediateResults.empty()) {
            // If we have no results, nothing to do
            cout << "No intermediate results to process" << endl;
            return;
        }
        
        // Join all intermediate results
        Relation joinedResult = intermediateResults[0];
        for (size_t i = 1; i < intermediateResults.size(); i++) {
            joinedResult = joinedResult.join(intermediateResults[i]);
        }

        // Now, we need to project and rename so the joined result matches the head predicate
        string headName = rule.getHeadPredicate().getName();
        if (!database.hasRelation(headName)) {
            cout << "Warning: Target relation " << headName << " does not exist" << endl;
            return;
        }

        const Relation& targetRelation = database.getRelation(headName);
        const Scheme& targetScheme = targetRelation.getScheme();
        
        // Get all variables from the head predicate
        vector<string> headVars;
        vector<size_t> projectIndices;
        
        // For each parameter in the head predicate
        for (const Parameter& param : rule.getHeadPredicate().getParameters()) {
            string varName = param.getValue();
            headVars.push_back(varName);
            
            // Find this variable in the joined result's scheme
            auto it = find(joinedResult.getScheme().begin(), joinedResult.getScheme().end(), varName);
            if (it != joinedResult.getScheme().end()) {
                size_t index = distance(joinedResult.getScheme().begin(), it);
                projectIndices.push_back(index);
            } else {
                cout << "Warning: Variable " << varName << " not found in joined result" << endl;
                // This is a critical error - we can't proceed if a head variable isn't in the result
                return;
            }
        }
        
        // Project and rename
        Relation projectedResult = joinedResult.project(projectIndices);
        Relation finalResult = projectedResult.rename(Scheme(headVars));
        
        // Now insert the tuples
        Relation& mutableTargetRelation = database.getRelation(headName);
        int beforeSize = mutableTargetRelation.getTuples().size();
        
        for (const Tuple& tuple : finalResult.getTuples()) {
            mutableTargetRelation.addTuple(tuple);
        }
        
        int afterSize = mutableTargetRelation.getTuples().size();
        cout << "Tuples added: " << (afterSize - beforeSize) << endl;
    }

    Relation applySelectionsAndProjections(Relation result, const Predicate& predicate) {
        map<string, size_t> variablePositions;
        vector<size_t> projectIndices;
        vector<string> variableNames;

        for (size_t i = 0; i < predicate.getParameters().size(); i++) {
            const Parameter& param = predicate.getParameters()[i];
            if (!param.getIsID()) {
                // This is a constant value (e.g., 'a')
                result = result.select(i, param.getValue());
            } else {
                // This is a variable (e.g., X)
                string varName = param.getValue();
                if (variablePositions.count(varName)) {
                    // We've seen this variable before, so select rows where both columns match
                    result = result.select(variablePositions[varName], i);
                } else {
                    // First time seeing this variable
                    variablePositions[varName] = i;
                    projectIndices.push_back(i);
                    variableNames.push_back(varName);
                }
            }
        }

        // Project only the columns we're interested in and rename them
        return result.project(projectIndices).rename(Scheme(variableNames));
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