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

  static bool joinable(const Scheme& leftScheme, const Scheme& rightScheme, const Tuple& leftTuple, const Tuple& rightTuple)
  {
    for (unsigned leftIndex = 0; leftIndex < leftScheme.size(); leftIndex++) 
    {
      const string& leftName = leftScheme.at(leftIndex);
      const string& leftValue = leftTuple.at(leftIndex);
      //cout << "left name: " << leftName << " value: " << leftValue << endl;
      for (unsigned rightIndex = 0; rightIndex < rightScheme.size(); rightIndex++)
      {
      const string& rightName = rightScheme.at(rightIndex);
      const string& rightValue = rightTuple.at(rightIndex);
      //cout << "right name: " << rightName << " value: " << rightValue << endl;

      if (leftName == rightName)
      {
        if (leftValue != rightValue)
        {
          return false;
        }
      }
      }
    }
    return true;
  }

  Relation join (const Relation& right)
  {
    const Relation& left = *this;
    //part 2 step 2;
    Scheme combinedScheme = left.scheme;
    for (const auto& attr : right.scheme)
    {
      if (find(combinedScheme.begin(), combinedScheme.end(), attr) == combinedScheme.end())
      {
        combinedScheme.push_back(attr);
      }
    }
    Relation result("joined_relation", combinedScheme);

    for (const auto& leftTuple : left.tuples)
    {
      //cout << "left tuple: " << leftTuple.toString(left.scheme) << endl;

      for (const auto& rightTuple : right.tuples)
      {
        //cout << "right tuple: " << rightTuple.toString(right.scheme) << endl;
        if (joinable(left.scheme, right.scheme, leftTuple, rightTuple))
        {
          vector<string> joinedValues;

          for (size_t i = 0; i < leftTuple.size(); i++)
          {
            joinedValues.push_back(leftTuple[i]);
          }

          for (size_t i = 0; i < right.scheme.size(); i++)
          {
            const string& rightAttr = right.scheme[i];
            const string& rightVal = rightTuple[i];
            
            bool found = false;

            for (size_t j = 0; j< left.scheme.size(); j++)
            {
              if (left.scheme[j] == rightAttr)
              {
                found = true;
                break;
              }
            }
            if (!found)
            {
              joinedValues.push_back(rightVal);
            }
          }
          Tuple joinedTuple(joinedValues);
          result.addTuple(joinedTuple);
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