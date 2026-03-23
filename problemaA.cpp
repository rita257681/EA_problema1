#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

using namespace std;

// Matriz com os 3 valores de cada trikit e os lados que eles podem conectar
struct Trikit {
    int v[3];
    int edges[3]; 
};

