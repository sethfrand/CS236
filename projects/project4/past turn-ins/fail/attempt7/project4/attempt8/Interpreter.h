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
    // Get the head predicate name and parameters
    string headName = rule.getHeadPredicate().getName();
    const vector<Parameter>& headParams = rule.getHeadPredicate().getParameters();
    
    // Process all body predicates
    Relation result;
    bool firstPredicate = true;
    
    for (const Predicate& bodyPredicate : rule.getBodyPredicates()) {
        if (!database.hasRelation(bodyPredicate.getName())) {
            cout << "Warning: Relation " << bodyPredicate.getName() << " does not exist" << endl;
            return; // Can't evaluate this rule
        }
        
        // Get a copy of the relation for this predicate
        Relation currentRelation = database.getRelation(bodyPredicate.getName());
        
        // Apply selections and projections - this is where the error might be
        Relation processedRelation = applySelectionsAndProjections(currentRelation, bodyPredicate);
        
        if (firstPredicate) {
            result = processedRelation;
            firstPredicate = false;
        } else {
            // The join operation might be causing the error
            try {
                result = result.join(processedRelation);
            } catch (const std::exception& e) {
                cout << "Error during join: " << e.what() << endl;
                return;
            }
        }
    }
    
    if (firstPredicate) {
        // No body predicates were processed
        return;
    }
    
    // Map head parameters to columns in the result
    vector<size_t> projectIndices;
    for (const Parameter& param : headParams) {
        string varName = param.getValue();
        
        // Find this variable in the result's scheme
        auto it = find(result.getScheme().begin(), result.getScheme().end(), varName);
        if (it != result.getScheme().end()) {
            size_t index = distance(result.getScheme().begin(), it);
            projectIndices.push_back(index);
        } else {
            cout << "Warning: Variable " << varName << " not found in result scheme" << endl;
            return; // Critical error
        }
    }
    
    // Project and rename with additional checks
    if (projectIndices.empty()) {
        cout << "Warning: No projection indices found for head predicate" << endl;
        return;
    }
    
    Relation projectedResult = result.project(projectIndices);
    
    // Create a scheme from the head parameters
    vector<string> headVars;
    for (const Parameter& param : headParams) {
        headVars.push_back(param.getValue());
    }
    
    if (headVars.empty()) {
        cout << "Warning: No variables in head predicate" << endl;
        return;
    }
    
    Relation finalResult = projectedResult.rename(Scheme(headVars));
    
    // Add tuples to the target relation
    Relation& targetRelation = database.getRelation(headName);
    int beforeSize = targetRelation.getTuples().size();
    
    for (const Tuple& tuple : finalResult.getTuples()) {
        targetRelation.addTuple(tuple);
    }
    
    int afterSize = targetRelation.getTuples().size();
    cout << "Tuples added: " << (afterSize - beforeSize) << endl;
}

Relation applySelectionsAndProjections(Relation result, const Predicate& predicate) {
    try {
        map<string, size_t> variablePositions;
        vector<size_t> projectIndices;
        vector<string> variableNames;
        
        for (size_t i = 0; i < predicate.getParameters().size(); i++) {
            const Parameter& param = predicate.getParameters()[i];
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
        
        // Skip projection and renaming if no variables found
        if (projectIndices.empty() || variableNames.empty()) {
            return result;
        }
        
        result = result.project(projectIndices);
        result = result.rename(Scheme(variableNames));
        return result;
    }
    catch (const std::exception& e) {
        cout << "Error in applySelectionsAndProjections: " << e.what() << endl;
        // Return the original relation in case of error
        return result;
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