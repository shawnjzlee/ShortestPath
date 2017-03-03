#include <fstream>
#include <sstream>
#include <string>
#include <climits>
#include <iostream>

#include "adjacencylist.h"
// #include "global.h"

using namespace std;

AdjacencyList::AdjacencyList(const string dataset) {
    ifstream instream;
    instream.open(dataset.c_str());
    if(!instream.is_open()) {
        throw std::runtime_error(string("Could not open file ") + dataset);
    }
    
    string line;
    int src, dst;
    while(getline(instream, line)) {
        if (line.at(0) == '#') continue;
        stringstream ss(line);
        ss >> src >> dst;
        
        insert_edge(src, dst);
    }
}

void AdjacencyList::insert_edge(int src, int dst) {
    if (src == dst) return;

    auto temp = (src < dst ? dst : src);
    
    if(incoming_edges.empty()) {
        incoming_edges.resize(temp + 1, vector<int>());
        outgoing_edges.resize(temp + 1, vector<int>());
        vertex_weight.resize(temp + 1, INT_MAX);
    } 
    else if (incoming_edges.size() <= src || incoming_edges.size() <= dst) {
        incoming_edges.resize(temp + 1, vector<int>());
        outgoing_edges.resize(temp + 1, vector<int>());
        vertex_weight.resize(temp + 1, INT_MAX);
    }
    
    incoming_edges.at(dst).push_back(src);
    outgoing_edges.at(src).push_back(dst);
}

void AdjacencyList::print_vertex_ranks() {
    for(int i = 0; i < vertex_weight.size(); i++)
        cout << i << "\t" << vertex_weight.at(i) << endl;
    cout << endl;
}