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
    if (tuple.size() != scheme.size()) {
        cerr << "Tuple size mismatch: Expected " << scheme.size() 
             << ", got " << tuple.size() << endl;
        return false;
    }
    // Return true if the tuple was inserted (i.e., it was new)
    return tuples.insert(tuple).second; 
}

  string toString() const {
    stringstream out;
    for (const auto& tuple : tuples) {
      for (size_t i = 0; i < scheme.size(); i++) {
        out << scheme[i] << "=" << tuple[i];
        if (i < scheme.size() - 1) out << ", ";
      }
      out << endl;
    }
    return out.str();
  }

  Relation select(int index, const string& value) const {
    Relation result(name, scheme);
    for (const auto& tuple : tuples) {
      if (tuple.at(index) == value) result.addTuple(tuple);
    }
    return result;
  }

  Relation select(int index1, int index2) const {
    Relation result(name, scheme);
    for (const auto& tuple : tuples) {
      if (tuple.at(index1) == tuple.at(index2)) result.addTuple(tuple);
    }
    return result;
  }

  Relation project(const vector<size_t>& columns) const {
    Scheme newScheme;
    for (size_t colIndex : columns) newScheme.push_back(scheme[colIndex]);
    Relation result(name, newScheme);
    for (const auto& tuple : tuples) {
      vector<string> newValues;
      for (size_t colIndex : columns) newValues.push_back(tuple[colIndex]);
      result.addTuple(Tuple(newValues));
    }
    return result;
  }

  Relation rename(const Scheme& newScheme) {
    Relation result(name, newScheme);
    for (const auto& tuple : tuples) result.addTuple(tuple);
    return result;
  }

  static bool joinable(const Scheme& leftScheme, const Scheme& rightScheme, 
                     const Tuple& leftTuple, const Tuple& rightTuple) {
    std::map<std::string, size_t> rightMap;
    for (size_t i = 0; i < rightScheme.size(); ++i) {
        rightMap[rightScheme[i]] = i;
    }

    for (size_t leftIndex = 0; leftIndex < leftScheme.size(); ++leftIndex) {
        const std::string& leftName = leftScheme[leftIndex];
        auto it = rightMap.find(leftName);
        if (it != rightMap.end()) {
            if (leftTuple[leftIndex] != rightTuple[it->second]) {
                return false;
            }
        }
    }
    return true;
}

Relation join(const Relation& right) const {
    const Relation& left = *this;
    Scheme combinedScheme = left.scheme;
    std::vector<size_t> rightIndices;

    // Build combined scheme by checking attribute names in Scheme (vector<string>)
    for (size_t i = 0; i < right.scheme.size(); ++i) {
        const std::string& attr = right.scheme[i];  // Access via vector index
        bool exists = false;
        for (const auto& lAttr : combinedScheme) {
            if (lAttr == attr) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            combinedScheme.push_back(attr);
            rightIndices.push_back(i);
        }
    }

    Relation result;
    result.scheme = combinedScheme;

    // Join tuples using existing helper
    for (const auto& lt : left.tuples) {
        for (const auto& rt : right.tuples) {
            if (joinable(left.scheme, right.scheme, lt, rt)) {
                // Directly use the Tuple (inherited vector<string>)
                std::vector<std::string> combined = lt;  // Tuple is a vector<string>

                // Append non-overlapping values from right tuple
                for (size_t i : rightIndices) {
                    combined.push_back(rt[i]);  // Access via vector index
                }
                result.tuples.insert(Tuple(combined));
            }
        }
    }
    return result;
}

  const string& getName() const { return name; }
  const Scheme& getScheme() const { return scheme; }
  const set<Tuple>& getTuples() const { return tuples; }

bool Union(const Relation& other) {
    // Check if schemes are compatible
    //if (scheme != other.scheme) return false;

    size_t before = tuples.size();
    tuples.insert(other.tuples.begin(), other.tuples.end());
    return tuples.size() > before;
}
};