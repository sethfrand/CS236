#pragma once
#include <map>
#include <set>
#include <stack>
#include <vector>
#include "Node.h"

class Graph {
private:
    std::map<int, Node> nodes;

public:
    Graph(int size) {
        for (int nodeID = 0; nodeID < size; nodeID++) {
            nodes[nodeID] = Node();
        }
    }

    const std::map<int, Node>& getNodes() const { return nodes; }

    void addEdge(int fromNodeID, int toNodeID) {
        nodes[fromNodeID].addEdge(toNodeID);
    }

    void resetVisited() {
        for (auto& pair : nodes) {
            pair.second.setVisited(false);
        }
    }

    std::stack<int> dfsForestPostOrder() {
        resetVisited();
        std::stack<int> postOrder;
        for (const auto& pair : nodes) {
            int nodeID = pair.first;
            if (!pair.second.isVisited()) {
                dfsPostOrder(nodeID, postOrder);
            }
        }
        return postOrder;
    }

    void dfsPostOrder(int nodeID, std::stack<int>& postOrder) {
        Node& node = nodes[nodeID];
        if (node.isVisited()) return;
        node.setVisited(true);
        for (int neighbor : node.getAdjacentNodeIDs()) {
            if (!nodes[neighbor].isVisited()) {
                dfsPostOrder(neighbor, postOrder);
            }
        }
        postOrder.push(nodeID);
    }

    std::vector<std::set<int>> findSCCs(std::stack<int>& order) {
        resetVisited();
        std::vector<std::set<int>> sccs;
        while (!order.empty()) {
            int nodeID = order.top();
            order.pop();
            if (!nodes[nodeID].isVisited()) {
                std::set<int> scc;
                dfsForSCC(nodeID, scc);
                sccs.push_back(scc);
            }
        }
        return sccs;
    }

    void dfsForSCC(int nodeID, std::set<int>& scc) {
        Node& node = nodes[nodeID];
        if (node.isVisited()) return;
        node.setVisited(true);
        scc.insert(nodeID);
        for (int neighbor : node.getAdjacentNodeIDs()) {
            if (!nodes[neighbor].isVisited()) {
                dfsForSCC(neighbor, scc);
            }
        }
    }

    Graph reversegraph() const {
        Graph reversed(nodes.size());
        for (const auto& pair : nodes) {
            int from = pair.first;
            for (int to : pair.second.getAdjacentNodeIDs()) {
                reversed.addEdge(to, from);
            }
        }
        return reversed;
    }

    std::string toString() 
    {
        std::string output;
        for (const auto& pair : nodes) {
            int nodeID = pair.first;
            const Node& node = pair.second;
            output += "R" + std::to_string(nodeID) + ": " + node.toString() + "\n";
        }
        return output;
    }
};