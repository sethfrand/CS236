#include "Scanner.h"
#include "Parser.h"
#include "Relation.h"
#include "Database.h"
#include "Interpreter.h"
#include <iostream>
#include <fstream>
#include <string>

int main() {
  Relation studentRelation("students", Scheme( {"ID", "Name", "Major"} ));

  vector<string> studentValues[] = {
    {"'42'", "'Ann'", "'CS'"},
    {"'64'", "'Ned'", "'EE'"},
  };

  vector<string> courseValues[] = {
    {"'42'", "'CS 100'"},
    {"'32'", "'CS 232'"},
  };

  for (auto& value : studentValues)
    studentRelation.addTuple(Tuple(value));

  Relation courseRelation("courses", Scheme( {"ID", "Course"} ));

  for (auto& value : courseValues)
    courseRelation.addTuple(Tuple(value));

  studentRelation.join(courseRelation);
}