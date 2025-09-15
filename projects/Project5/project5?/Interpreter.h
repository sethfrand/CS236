#pragma once
#include "DatalogProgram.h"
#include "Database.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>
#include <sstream>
#include <set>
#include "graph.h"

class Interpreter {
private:
    DatalogProgram program;
    Database db;

public:
    explicit Interpreter(const DatalogProgram& prog) : program(prog) {}

    void run() {
        std::cout << "Dependency Graph" << std::endl;
        evaluateSchemes();
        evaluateFacts();
        Graph DependencyGraph = makeGraph(program.getRules());
        Graph ReverseGraph = DependencyGraph.reversegraph();
        std::stack<int> postOrder = ReverseGraph.dfsForestPostOrder();
        std::vector<std::set<int>> SCCs = DependencyGraph.findSCCs(postOrder);
        evaluateRulesWithSCC(SCCs);
        evaluateQueries();
    }

    void evaluateRulesWithSCC(const std::vector<std::set<int>>& SCCs) 
    {
        std::cout << "Rule Evaluation" << std::endl;
        for (const auto& scc : SCCs)
        {
            std::vector<int> sccVector(scc.begin(), scc.end());
            std::sort(sccVector.begin(), sccVector.end());
            std::cout << "SCC: ";
            for (size_t i = 0; i < sccVector.size(); i++)
            {
                std::cout <<"R" << sccVector[i];
                if (i < sccVector.size() - 1)
                {
                    std::cout << ",";
                }
                else 
                {
                    std::cout << "" << std::endl;
                }
            }

            bool triv = sccVector.size() == 1;
            
            // For a single rule SCC
            if (triv)
            {
                int ruleIndex = sccVector[0];
                const Rule& rule = program.getRules()[ruleIndex];
                
                // Print the rule once
                std::cout << rule.toString() << "." << std::endl;
                
                // Now evaluate the rule without printing
                std::vector<int> rulesToEvaluate = {ruleIndex};
                int passes = evaluateRules(rulesToEvaluate, false);
                
                std::cout << passes << " passes: ";
                for (size_t i = 0; i < sccVector.size(); i++)
                {
                    std::cout << "R" << sccVector[i] << (i < sccVector.size() - 1 ? "," : "");
                }
                //std::cout << "\n";
            } else {
                // For multiple rule SCCs
                int passes = evaluateRules(sccVector, true);
                
                std::cout << passes << " passes: ";
                for (size_t i = 0; i < sccVector.size(); i++)
                {
                    std::cout << "R" << sccVector[i] << (i < sccVector.size() - 1 ? "," : "");
                }
                std::cout << "\n";
            }

            std::cout << "\n";
        }
    }

    static Graph makeGraph(const std::vector<Rule>& rules)
    {
        Graph graph(rules.size());

        // First loop: Iterate over each rule (from rules)
        for (size_t i = 0; i < rules.size(); i++) 
        {
            const Rule& fromRule = rules[i];
            //std::cout << "from rule R" << i << ": " << fromRule.toString() << std::endl;

            // Second loop: Iterate over body predicates of the current rule
            for (const Predicate& bodyPredicate : fromRule.getBodyPredicates()) 
            {
                //std::cout << "from body predicate: " << bodyPredicate.toString() << std::endl;

                // Third loop: Iterate over all rules again (to rules)
                for (size_t j = 0; j < rules.size(); j++) 
                {
                    const Rule& toRule = rules[j];
                    //std::cout << "to rule R" << j << ": " << toRule.toString() << std::endl;

                    // Check if body predicate matches the head predicate of the 'to' rule
                    if (bodyPredicate.getName() == toRule.getHeadPredicate().getName()) 
                    {
                        // Add edge from i (fromRule) to j (toRule)
                        graph.addEdge(i, j);
                    }
                }
            }
        }

        // Print the graph's adjacency list
        for (const auto& pair : graph.getNodes())  
        {
            int nodeID = pair.first;
            const Node& node = pair.second;

            std::cout << "R" << nodeID << ":";
            
            // Get the adjacent nodes and print them directly
            const std::set<int>& adjacentNodes = node.getAdjacentNodeIDs();
            if (!adjacentNodes.empty()) {
                bool first = true;
                for (int adjNode : adjacentNodes) {
                    if (!first) {
                        std::cout << ",";
                    }
                    std::cout << "R" << adjNode;
                    first = false;
                }
            }
            
            std::cout << std::endl;
        }
        std::cout << std::endl;
        return graph;
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
        std::cout << std::endl;
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

    // Modified to accept a printRules parameter
    int evaluateRules(const std::vector<int>& ruleIndices = std::vector<int>(), bool printRules = true) {
        int passes = 0;
        bool changed;
        
        // If no indices specified, use all rules
        bool evaluateAllRules = ruleIndices.empty();
        
        // Get the indices of all rules if needed
        std::vector<int> indicesToUse;
        if (evaluateAllRules) {
            for (size_t i = 0; i < program.getRules().size(); i++) {
                indicesToUse.push_back(i);
            }
        } else {
            indicesToUse = ruleIndices; // Use the specified indices
        }

        do {
            changed = false;
            passes++;

            for (int ruleIndex : indicesToUse) {
                const Rule& rule = program.getRules()[ruleIndex];
                if (printRules) {
                    std::cout << rule.toString() << "." << std::endl;
                }

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

                // Process head-predicate
                const auto& headPredicate = rule.getHeadPredicate();
                std::vector<size_t> headProjectIndices;
                std::vector<std::string> outputVarNames;

                for (const auto& headParam : headPredicate.getParameters()) {
                    outputVarNames.push_back(headParam.getValue());
                }

                // Find position of head variables in the results scheme
                for (const auto& varName : outputVarNames) {
                    for (size_t i = 0; i < result.getScheme().size(); ++i) {
                        if (result.getScheme()[i] == varName) {
                            headProjectIndices.push_back(i);
                            break;
                        }
                    }
                }

                result = result.project(headProjectIndices);

                // Rename to match the TARGET relation's scheme
                Relation& target = db.getRelation(headPredicate.getName());
                result = result.rename(target.getScheme());

                // Collect new tuples and print using TARGET's scheme
                std::vector<Tuple> newTuples;
                for (const auto& t : result.getTuples()) {
                    if (target.addTuple(t)) {
                        newTuples.push_back(t);
                    }
                }

                // Print tuples with TARGET's attribute names
                const Scheme& targetScheme = target.getScheme();
                for (const auto& t : newTuples) {
                    std::cout << "  ";
                    for (size_t i = 0; i < targetScheme.size(); ++i) {
                        std::cout << targetScheme[i] << "='" << t[i] << "'";
                        if (i < targetScheme.size() - 1) std::cout << ", ";
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