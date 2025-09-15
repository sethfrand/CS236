#pragma once
#include "DatalogProgram.h"
#include "Database.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>
#include <sstream>

class Interpreter {
private:
    DatalogProgram program;
    Database db;

public:
    explicit Interpreter(const DatalogProgram& prog) : program(prog) {}

    void run() {
        evaluateSchemes();
        evaluateFacts();
       // std::cout << "Rule Evaluation" << std::endl;
        int passes = evaluateRules();
        
        std::cout << std::endl << "Schemes populated after " << passes << " passes through the Rules." << std::endl << std::endl;
        evaluateQueries();
    }

private:
    void evaluateSchemes() {
        for (const auto& scheme : program.getSchemes()) {
            std::vector<std::string> attributes;
            auto params = scheme.getParameters();
            for (const auto& param : params) {
                attributes.push_back(param.getValue());
            }
            db.createRelation(scheme.getName(), Scheme(attributes));
        }
    }

    void evaluateFacts() {
        for (const auto& fact : program.getFacts()) {
            std::vector<std::string> values;
            auto params = fact.getParameters();
            for (const auto& param : params) {
                std::string val = param.getValue();
                if (val.size() >= 2 && val.front() == '\'' && val.back() == '\'') {
                    val = val.substr(1, val.size() - 2);
                }
                values.push_back(val);
            }
            db.getRelation(fact.getName()).addTuple(Tuple(values));
        }
    }

    void evaluateQueries() {
        std::cout << "Query Evaluation" << std::endl;
        for (const auto& query : program.getQueries()) {
            Relation result = db.getRelation(query.getName());
            std::map<std::string, int> varPositions;
            std::vector<size_t> projectIndices;
            std::vector<std::string> renameList;

            auto params = query.getParameters();
            for (size_t i = 0; i < params.size(); ++i) {
                const auto& param = params[i];
                if (!param.getIsID()) {
                    std::string value = param.getValue();
                    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                        value = value.substr(1, value.size() - 2);
                    }
                    result = result.select(i, value);
                } else {
                    const std::string& varName = param.getValue();
                    if (varPositions.find(varName) != varPositions.end()) {
                        result = result.select(varPositions[varName], i);
                    } else {
                        varPositions[varName] = i;
                        projectIndices.push_back(i);
                        renameList.push_back(varName);
                    }
                }
            }

            result = result.project(projectIndices);
            result = result.rename(renameList);

            std::cout << query.toString() << "? ";
            if (result.getTuples().empty()) {
                std::cout << "No" << std::endl;
            } else {
                std::cout << "Yes(" << result.getTuples().size() << ")" << std::endl;
                std::vector<Tuple> sortedTuples(result.getTuples().begin(), result.getTuples().end());
                std::sort(sortedTuples.begin(), sortedTuples.end());

                for (const auto& t : sortedTuples) {
                    std::cout << "  ";
                    for (size_t i = 0; i < renameList.size(); ++i) {
                        std::cout << renameList[i] << "='" << t[i] << "'";
                        if (i < renameList.size() - 1) std::cout << ", ";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }

int evaluateRules() {
    int passes = 0;
    const auto& rules = program.getRules();
    bool changed;

    std::cout << "Rule Evaluation" << std::endl;

    do {
        changed = false;
        passes++;
        for (const auto& rule : rules) {
            std::cout << rule.toString() << "." << std::endl;

            std::vector<Relation> intermediates;

            // Process body predicates
            for (const auto& bodyPred : rule.getBodyPredicates()) {
                Relation r = db.getRelation(bodyPred.getName());
                auto params = bodyPred.getParameters();

                std::map<std::string, int> varIndices;
                std::vector<size_t> projectIndices;
                std::vector<std::string> renameAttrs;

                for (size_t i = 0; i < params.size(); ++i) {
                    const auto& param = params[i];
                    if (!param.getIsID()) {
                        std::string value = param.getValue();
                        if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                            value = value.substr(1, value.size() - 2);
                        }
                        r = r.select(i, value);
                    } else {
                        const std::string& varName = param.getValue();
                        if (varIndices.find(varName) != varIndices.end()) {
                            r = r.select(varIndices[varName], i);
                        } else {
                            varIndices[varName] = i;
                            projectIndices.push_back(i);
                            renameAttrs.push_back(varName);
                        }
                    }
                }

                r = r.project(projectIndices);
                r = r.rename(renameAttrs);
                intermediates.push_back(r);
            }

            if (intermediates.empty()) continue;
            Relation result = intermediates[0];
            for (size_t i = 1; i < intermediates.size(); ++i) {
                result = result.join(intermediates[i]);
            }

            // Process head predicate
            const auto& headPredicate = rule.getHeadPredicate();
            Relation& target = db.getRelation(headPredicate.getName());
            const Scheme& targetScheme = target.getScheme();

            // Collect head parameter names in order
            std::vector<std::string> headParamNames;
            for (const auto& param : headPredicate.getParameters()) {
                headParamNames.push_back(param.getValue());
            }

            // Determine projection indices based on head parameter order
            std::vector<size_t> headProjectIndices;
            for (const auto& paramName : headParamNames) {
                bool found = false;
                for (size_t i = 0; i < result.getScheme().size(); ++i) {
                    if (result.getScheme()[i] == paramName) {
                        headProjectIndices.push_back(i);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    // This should not happen if the rule is valid
                    throw std::runtime_error("Head parameter not found in result scheme");
                }
            }

            // Project and rename to target scheme
            result = result.project(headProjectIndices);
            result = result.rename(targetScheme);

            // Collect new tuples
            std::vector<Tuple> newTuples;
            for (const auto& t : result.getTuples()) {
                if (target.addTuple(t)) {
                    newTuples.push_back(t);
                }
            }

            // Print new tuples using target's scheme
            const Scheme& printScheme = target.getScheme();
            std::sort(newTuples.begin(), newTuples.end());
            for (const auto& tuple : newTuples) {
                std::cout << "  ";
                for (size_t i = 0; i < printScheme.size(); ++i) {
                    std::cout << printScheme[i] << "='" << tuple[i] << "'";
                    if (i < printScheme.size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }

            if (!newTuples.empty()) {
                changed = true;
            }
        }
    } while (changed);

    return passes;
}
};