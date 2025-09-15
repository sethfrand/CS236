#pragma once
#include <map>
#include "Node.h"
#include <string>


class Graph
{
    Node node; 
    private: 
        std::map<int, Node> nodes;
        
    public:
    const std::map<int, Node>& getNodes() const { return nodes; }

        Graph(int size) 
        {
            for (int nodeID = 0; nodeID < size; nodeID++)
            {
                nodes[nodeID] = Node();
            }
        }

        void addEdge(int fromNodeID, int toNodeID)
        {
            nodes[fromNodeID].addEdge(toNodeID);
        }
    
    string toString()
    {
        std::string output;
        for (const auto& pair : nodes)
        {
            int nodeID = pair.first;
            Node node = pair.second;
            output += "R" + std::to_string(nodeID) + ": " + node.toString() + "\n";
        }
        return output;
    }


};