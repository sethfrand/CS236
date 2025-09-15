#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Scheme.h"
#include "Tuple.h"
#include <set>
#include <algorithm>

using namespace std; 

class Relation {

 private:
  string name;
  Scheme scheme;
  set<Tuple> tuples;

 public:
  Relation () {}
  Relation(const string& name, const Scheme& scheme)
    : name(name), scheme(scheme) { }

  void addTuple(const Tuple& tuple) 
  {
    // Check tuple size
    if (tuple.size() != scheme.size()) {
        cerr << "Tuple size mismatch: Expected " << scheme.size() 
             << ", got " << tuple.size() << endl;
        return;
    }

    // Check if tuple is actually new
    tuples.insert(tuple);
  }

  string toString() const 
  {
    stringstream out;
    for (const auto& tuple : tuples)
    {
        for (size_t i = 0; i < scheme.size(); i++)
        {
          out << scheme[i] << "=" << tuple[i];
          if (i < scheme.size() - 1)
          {
            out << ", ";
          }
        }
        out << endl;
    }
    return out.str();
  }

  Relation select(int index, const string& value) const // for checking one column
  {
    Relation result(name, scheme);
    for (const auto& tuple : tuples)
    {
        if (tuple.at(index) == value)
        {
            result.addTuple(tuple);
        }
    }
    return result;
  }

  Relation select(int index1, int index2) const // for checking 2 columns
  {
    Relation result(name, scheme);
    for (const auto& tuple : tuples)
    {
        if (tuple.at(index1) == tuple.at(index2))
        {
            result.addTuple(tuple);
        }
    }
    return result;
  }

  //project
  Relation project(const vector<size_t>& columns) const
  {
    Scheme newScheme;
    for (int colIndex : columns)
    {
      newScheme.push_back(scheme[colIndex]);
    }

    Relation result(name, newScheme);
    for (const auto& tuple : tuples)
    {
      vector<string>newValues;
      for (int colIndex : columns)
      {
        newValues.push_back(tuple[colIndex]);
      }
      Tuple newTuple(newValues);
      result.addTuple(newTuple);
    }
    return result;
  }

  //rename
  Relation rename(const Scheme& newScheme)
  {
    Relation result(name, newScheme);
    for (const auto& tuple : tuples)
    {
      result.addTuple(tuple);
    }
    return result; 
  }

  Relation join(const Relation& right)
  {
    const Relation& left = *this;
    
    // Compute column mapping and combined scheme
    vector<size_t> leftJoinColumns;
    vector<size_t> rightJoinColumns;
    Scheme combinedScheme;
    
    // First, add all columns from left scheme
    for (size_t i = 0; i < left.scheme.size(); ++i) {
        combinedScheme.push_back(left.scheme[i]);
    }
    
    // Find matching columns and add non-matching columns from right
    for (size_t j = 0; j < right.scheme.size(); ++j) {
        bool found = false;
        for (size_t i = 0; i < left.scheme.size(); ++i) {
            if (left.scheme[i] == right.scheme[j]) {
                leftJoinColumns.push_back(i);
                rightJoinColumns.push_back(j);
                found = true;
                break;
            }
        }
        
        if (!found) {
            combinedScheme.push_back(right.scheme[j]);
        }
    }
    
    // Create result relation
    Relation result("joined_relation", combinedScheme);
    
    // Perform join
    for (const auto& leftTuple : left.tuples) {
        for (const auto& rightTuple : right.tuples) {
            // Check if tuples are joinable on common columns
            bool canJoin = true;
            for (size_t k = 0; k < leftJoinColumns.size(); ++k) {
                if (leftTuple[leftJoinColumns[k]] != rightTuple[rightJoinColumns[k]]) {
                    canJoin = false;
                    break;
                }
            }
            
            // If joinable, create combined tuple
            if (canJoin) {
                vector<string> joinedValues;
                
                // Add left tuple values
                joinedValues.insert(joinedValues.end(), 
                                    leftTuple.begin(), 
                                    leftTuple.end());
                
                // Add right tuple values for non-matching columns
                for (size_t j = 0; j < right.scheme.size(); ++j) {
                    bool isMatchingColumn = false;
                    for (size_t k = 0; k < rightJoinColumns.size(); ++k) {
                        if (rightJoinColumns[k] == j) {
                            isMatchingColumn = true;
                            break;
                        }
                    }
                    
                    if (!isMatchingColumn) {
                        joinedValues.push_back(rightTuple[j]);
                    }
                }
                
                result.addTuple(Tuple(joinedValues));
            }
        }
    }
    
    return result;
  }

  const string& getName() const
  {
    return name;
  }

  const Scheme& getScheme() const
  {
    return scheme;
  }

  const set<Tuple>& getTuples() const
  {
    return tuples;
  }
};