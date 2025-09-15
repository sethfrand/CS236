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
        vector<Relation> intermediateResults;
        const vector<Predicate>& bodyPredicates = rule.getBodyPredicates();

        Relation& targetRelation = database.getRelation(headName);
        lastResultRelation = Relation(headName, targetRelation.getScheme());

        for (const Predicate& bodyPredicate : bodyPredicates) {
            string relationName = bodyPredicate.getName();
            if (!database.hasRelation(relationName)) {
                continue;
            }

            Relation result = database.getRelation(relationName);
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

        Relation projected = result.project(projectIndices);
        Relation renamed = projected.rename(Scheme(variableNames));
        intermediateResults.push_back(renamed);
    }

    Relation joinedResult;
    if (!intermediateResults.empty()) {
        joinedResult = intermediateResults[0];
        for (size_t i = 1; i < intermediateResults.size(); i++) {
            joinedResult = joinedResult.join(intermediateResults[i]);
        }

        const Scheme& expectedScheme = database.getRelation(headName).getScheme();
        vector<size_t> finalProjectIndices;
        for (const string& attr : expectedScheme) {
            for (size_t j = 0; j < joinedResult.getScheme().size(); j++) {
                if (joinedResult.getScheme()[j] == attr) {
                    finalProjectIndices.push_back(j);
                    break;
                }
            }
        }

        joinedResult = joinedResult.project(finalProjectIndices).rename(expectedScheme);
    }

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