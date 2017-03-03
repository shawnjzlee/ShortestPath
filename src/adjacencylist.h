#ifndef ADJACENCY_H
#define ADJACENCY_H

#include <vector> 
#include <functional>

using namespace std;

class AdjacencyList {
    public:
        vector <vector<int>> incoming_edges;
        vector <vector<int>> outgoing_edges;
        vector <int> vertex_weight;
        
        void print_vertex_ranks();
    
    public:
        AdjacencyList() {};
        ~AdjacencyList() {};
        AdjacencyList(const string dataset);
};

#endif /* adjacencylist.h */