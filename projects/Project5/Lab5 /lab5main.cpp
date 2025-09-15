#include "Scanner.h"
#include "Parser.h"
#include "Relation.h"
#include "Database.h"
#include "Interpreter.h"
#include "Node.h"
#include "graph.h"
#include <iostream>
#include <fstream>
#include <string>



int main()
{
    // Node node;
    // node.addEdge(4);
    // node.addEdge(8);
    // node.addEdge(2);
    // cout << node.toString() << endl;

    // Graph graph(3);
    // graph.addEdge(1,2);
    // graph.addEdge(1,0);
    // graph.addEdge(0,1);
    // graph.addEdge(1,1);
    // cout << graph.toString() << endl;

    //predicate names for fake rules
    //first is name for head predicate
    //second is names for body predicates
  pair<string,vector<string>> ruleNames[] = {
    { "A", { "B", "C" } },
    { "B", { "A", "D" } },
    { "B", { "B" } },
    { "E", { "F", "G" } },
    { "E", { "E", "F" } },
  };

        vector<Rule> rules;
        for (auto& rulePair :ruleNames)
        {
            string headName = rulePair.first;
            Rule rule = Rule(Predicate(headName));
            vector<string> bodyNames = rulePair.second;
            for (auto& bodyName : bodyNames)
            {
                rule.addBodyPredicate(Predicate(bodyName));
            }
            rules.push_back(rule);
        }
        Graph graph = Interpreter::makeGraph(rules);
        cout << graph.toString() << endl;
    return 0;
}

