#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Tuple.h"

using namespace std;


class Scheme: public vector<string> 
{
    public:
    Scheme(vector<string> names) : vector<string>(names) { }

};