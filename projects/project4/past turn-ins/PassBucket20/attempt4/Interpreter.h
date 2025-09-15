#pragma once
#include "DatalogProgram.h"
#include "Database.h"
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <stdexcept>

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
            
            if (!database.hasRelation(relationName)) {
                throw runtime_error("Relation '" + relationName + "' not found in the database.");
            }
            
            vector<string> values;
            for (const Parameter& param : factPredicate.getParameters()) {
                values.push_back(param.getValue());
            }

            Relation& relation = database.getRelation(relationName);
            if (values.size() != relation.getScheme().size()) {
                throw runtime_error("Tuple size mismatch for relation '" + relationName + "'");
            }

            relation.addTuple(Tuple(values));
        }
    }

    string evalRules() {
        stringstream ss;
        const vector<Rule>& rules = datalogprogram.getRules();

        ss << "Rule Evaluation" << endl;
        
        if (rules.empty()) {
            ss << "\nSchemes populated after 1 passes through the Rules.\n\n";
            ss << "Query Evaluation" << endl;
            return ss.str();
        }

        int numPasses = 0;
        bool tuplesAdded;

        do {
            tuplesAdded = false;
            numPasses++;

            for (const Rule& rule : rules) {
                ss << rule.toString() << "." << endl;
                
                lastResultRelation = Relation(); 
                int tupleAddedByRule = evalRule(rule);

                if (tupleAddedByRule > 0) {
                    tuplesAdded = true;
                    for (const Tuple& tuple : lastResultRelation.getTuples()) {
                        ss << "  "; 
                        for (size_t i = 0; i < lastResultRelation.getScheme().size(); i++) {
                            string value = tuple[i];
                            if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                                value = value.substr(1, value.length() - 2);
                            }
                            
                            ss << lastResultRelation.getScheme()[i] << "='" << value << "'";
                            if (i < lastResultRelation.getScheme().size() - 1) {
                                ss << ", ";
                            }
                        }
                        ss << endl;
                    }
                }
            }
        } while (tuplesAdded);

        ss << "\nSchemes populated after " << numPasses << " passes through the Rules.\n\n";
        ss << "Query Evaluation" << endl;

        return ss.str();
    }

int evalRule(const Rule& rule) {
    const Predicate& headPredicate = rule.getHeadPredicate();
    string headName = headPredicate.getName();
    const vector<Predicate>& bodyPredicates = rule.getBodyPredicates();

    Relation& targetRelation = database.getRelation(headName);
    lastResultRelation = Relation(headName, targetRelation.getScheme());

    // Prepare intermediate results
    vector<Relation> intermediateResults;

    // Process each body predicate
    for (const Predicate& bodyPredicate : bodyPredicates) {
        string relationName = bodyPredicate.getName();
        
        if (!database.hasRelation(relationName)) {
            continue;
        }

        Relation result = database.getRelation(relationName);
        const auto& parameters = bodyPredicate.getParameters();
        
        // Apply constant selections first
        for (size_t i = 0; i < parameters.size(); i++) {
            if (!parameters[i].getIsID()) {
                result = result.select(i, parameters[i].getValue());
            }
        }

        intermediateResults.push_back(result);
    }

    // Join intermediate results
    Relation joinedResult;
    if (!intermediateResults.empty()) {
        joinedResult = intermediateResults[0];
        for (size_t i = 1; i < intermediateResults.size(); i++) {
            joinedResult = joinedResult.join(intermediateResults[i]);
        }

        // Prepare for final projection
        const Scheme& expectedScheme = targetRelation.getScheme();
        vector<size_t> finalProjectIndices;
        
        // Map variables from head predicate to join result
        map<string, size_t> variableMap;
        for (const Predicate& bodyPredicate : bodyPredicates) {
            const auto& parameters = bodyPredicate.getParameters();
            for (size_t i = 0; i < parameters.size(); i++) {
                if (parameters[i].getIsID()) {
                    string varName = parameters[i].getValue();
                    size_t colIndex = findColumnIndex(joinedResult.getScheme(), 
                        bodyPredicate.getName(), i);
                    if (colIndex != string::npos) {
                        variableMap[varName] = colIndex;
                    }
                }
            }
        }

        // Find projection indices based on head predicate
        for (const string& headAttr : expectedScheme) {
            auto it = find_if(variableMap.begin(), variableMap.end(), 
                [&headAttr](const pair<string, size_t>& p) { 
                    return p.first == headAttr; 
                });
            
            if (it != variableMap.end()) {
                finalProjectIndices.push_back(it->second);
            }
        }

        // Project and rename to match target relation
        if (!finalProjectIndices.empty()) {
            joinedResult = joinedResult.project(finalProjectIndices).rename(expectedScheme);
        }
    }

    // Add tuples to target relation
    int tuplesAdded = 0;
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

// Helper function to find column index
size_t findColumnIndex(const Scheme& scheme, const string& relationName, size_t paramIndex) {
    for (size_t i = 0; i < scheme.size(); ++i) {
        // This is a simplified matching - you might need to adjust based on your exact naming convention
        if (scheme[i].find(relationName) != string::npos) {
            return i;
        }
    }
    return string::npos;
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
                        result = result.select(i, param.getValue());
                    } else {
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
                        Relation renamed = result.project(projectIndices).rename(Scheme(variableNames));
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
    Relation lastResultRelation;
};
