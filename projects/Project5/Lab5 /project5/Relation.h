#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Scheme.h"
#include "Tuple.h"
#include <set>
#include <algorithm>
#include <map>  

using namespace std; 

class Relation {
 private:
  string name;
  Scheme scheme;
  set<Tuple> tuples;

 public:
  Relation() {}
  Relation(const string& name, const Scheme& scheme) : name(name), scheme(scheme) { }

  bool addTuple(const Tuple& tuple) {
    return tuples.insert(tuple).second; 
  }

//select methods
  Relation select(int index, const string& value) const {
    Relation result(name, scheme);
    for (const auto& tuple : tuples) {
      if (tuple[index] == value) result.addTuple(tuple);
    }
    return result;
  }
//select fro two indexes
  Relation select(int index1, int index2) const {
    Relation result(name, scheme);
    for (const auto& tuple : tuples) {
      if (tuple[index1] == tuple[index2]) result.addTuple(tuple);
    }
    return result;
  }
//project
  Relation project(const vector<size_t>& columns) const {
    Scheme newScheme;
    for (size_t colIndex : columns) 
      newScheme.push_back(scheme[colIndex]);
    
    Relation result(name, newScheme);
    for (const auto& tuple : tuples) {
      vector<string> newValues;
      for (size_t colIndex : columns)
        newValues.push_back(tuple[colIndex]);
      result.addTuple(Tuple(newValues));
    }
    return result;
  }

  Relation rename(const Scheme& newScheme) {
    Relation result(name, newScheme);
    result.tuples = tuples;
    return result;
  }

  static bool joinable(const Scheme& leftScheme, const Scheme& rightScheme, 
                     const Tuple& leftTuple, const Tuple& rightTuple) {
    for (size_t leftIdx = 0; leftIdx < leftScheme.size(); leftIdx++) {
      for (size_t rightIdx = 0; rightIdx < rightScheme.size(); rightIdx++) {
        if (leftScheme[leftIdx] == rightScheme[rightIdx]) {
          if (leftTuple[leftIdx] != rightTuple[rightIdx]) {
            return false;
          }
        }
      }
    }
    return true;
  }

  Relation join(const Relation& right) const {
    const Relation& left = *this;
    Scheme combinedScheme = left.scheme;
    vector<pair<size_t, size_t>> overlap; // Positions of overlapping attributes

    // Ident overlapping attributes
    for (size_t i = 0; i < left.scheme.size(); i++) {
        for (size_t j = 0; j < right.scheme.size(); j++) {
            if (left.scheme[i] == right.scheme[j]) {
                overlap.emplace_back(i, j);
            }
        }
    }

    // Add non-overlapping attributes from right
    for (size_t j = 0; j < right.scheme.size(); j++) {
        bool isOverlap = false;
        for (const auto& pair : overlap) {
            if (j == pair.second) {
                isOverlap = true;
                break;
            }
        }
        if (!isOverlap) {
            combinedScheme.push_back(right.scheme[j]);
        }
    }

    // Join tuples
    Relation result(left.name + "-" + right.name, combinedScheme);
    for (const Tuple& lt : left.tuples) {
        for (const Tuple& rt : right.tuples) {
            bool canJoin = true;
            for (const auto& pair : overlap) {
                if (lt[pair.first] != rt[pair.second]) {
                    canJoin = false;
                    break;
                }
            }
            if (canJoin) {
                Tuple newTuple = lt;
                for (size_t j = 0; j < rt.size(); j++) {
                    bool isOverlap = false;
                    for (const auto& pair : overlap) {
                        if (j == pair.second) {
                            isOverlap = true;
                            break;
                        }
                    }
                    if (!isOverlap) {
                        newTuple.push_back(rt[j]);
                    }
                }
                result.addTuple(newTuple);
            }
        }
    }
    return result;
}

  //getters and Union method
  const string& getName() const { return name; }
  const Scheme& getScheme() const { return scheme; }
  const set<Tuple>& getTuples() const { return tuples; }

  bool Union(const Relation& other) {
    size_t before = tuples.size();
    tuples.insert(other.tuples.begin(), other.tuples.end());
    return tuples.size() > before;
  }
};