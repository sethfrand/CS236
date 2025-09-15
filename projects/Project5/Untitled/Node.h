#pragma once
#include <set>
#include <string>

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

    string toString() const
{
    string output;
    auto it = adjacentNodeIDs.begin();
    
    while (it != adjacentNodeIDs.end()) 
    {
        output += "R" + to_string(*it);  // Add node ID
        ++it;
        
        if (it != adjacentNodeIDs.end()) 
        {
            output += ", ";
        }
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
