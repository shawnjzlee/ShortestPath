#include <iostream>
#include <ios>
#include <fstream>
#include <iomanip>
#include <vector>
#include <sstream>

#include <thread>
#include <chrono>
#include <cstdio>
#include <functional>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <climits>

#include "adjacencylist.h"

using namespace std;
using namespace std::chrono;

void rank_distribution(AdjacencyList& graph) {
    graph.set_vertex_rank(0,0);
    vector <bool> discovered_vertex (graph.vertex_path_cost.size(), false);
    int difference = 1, max_difference = 1;
    while (max_difference > 0) {
        for (int i = 0; i < graph.outgoing_edges.size(); i++) {
            for_each(graph.outgoing_edges[i].begin(), graph.outgoing_edges[i].end(), [&](int &j) {
                int curr_path_cost = graph.vertex_path_cost.at(j);
                
                if (j == 0) return;
                
                if (discovered_vertex.at(j) == false) {
                    graph.set_vertex_rank(j, graph.vertex_path_cost.at(i) + 1);
                    discovered_vertex.at(j) = true;
                }
                else if (discovered_vertex.at(j) == true && graph.vertex_path_cost.at(i) + 1 <= curr_path_cost) {
                    graph.set_vertex_rank(j, graph.vertex_path_cost.at(i) + 1);
                }
                
                difference = abs(curr_path_cost - graph.vertex_path_cost.at(j));
                if (difference < max_difference) {
                    max_difference = difference;
                }
                
                graph.print_vertex_ranks();
            });
        }
    }
}

int main(int argc, char *argv[]) {
    string dataset = argv[1];
    AdjacencyList graph (dataset);
    
    rank_distribution(graph);
    graph.print_vertex_ranks();
}

