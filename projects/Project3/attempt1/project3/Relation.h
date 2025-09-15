#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Scheme.h"
#include "Tuple.h"
#include <set>

using namespace std; 

class Relation {

 private:

  string name;
  Scheme scheme;
  set<Tuple> tuples;

 public:

  Relation(const string& name, const Scheme& scheme)
    : name(name), scheme(scheme) { }

  void addTuple(const Tuple& tuple) {
    tuples.insert(tuple);
  }

  string toString() const 
  {
        stringstream out;
        for (const auto& tuple : tuples)
        {
            for (size_t i = 0; i<scheme.size(); i++)
            {
              out << scheme[i] << "=" << tuple[i] << "'";
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
  Relation project(const vector<int>& columns) const
  {
    Scheme newScheme;
    for (int colIndex : columns)
    {
      newScheme.pushback(scheme[colIndex]);
    }

    Relation result(name, newScheme)
    for (const auto& tuple : tuples)
    {
      vector<string>NewValues;
      for (int colIndex : columns)
      {
        NewValues.pushback(tuple[colIndex]);
      }
      Tuple newTuple(NewValues);
      result.addtuple(newTuple);
    }
    return result
  }

  //rename
  Relation rename(const Scheme& newScheme)
  {
    for (const auto& tuple : tuples)
    {
      result.addTuple(tuple);
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