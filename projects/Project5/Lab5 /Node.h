#pragma once
#include <set>
#include <string>

class Node 

{
    private:
        set<int> adjacentNodeIDs;
    public:
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

};
