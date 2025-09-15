#pragma once
#include <set>
#include <string>
#include <iostream>

class Node 
{
private:
    std::set<int> adjacentNodeIDs;
    bool visited = false;
    
public:
    const std::set<int>& getAdjacentNodeIDs() const 
    {
        return adjacentNodeIDs;
    }
    
    void addEdge(int adjacentnodeID) 
    {
        adjacentNodeIDs.insert(adjacentnodeID);
    }

    std::string toString() const
    {
        std::string output = "";
        bool first = true;
        
        for (int nodeID : adjacentNodeIDs) {
            if (!first) {
                output += ",";
            }
            output += "R" + std::to_string(nodeID);
            first = false;
        }
        
        return output;
    }

    bool isVisited() const    
    {
        return visited;
    }

    void setVisited(bool visited) 
    {
        this->visited = visited;
    }
};