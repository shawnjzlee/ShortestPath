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
    double difference = 1.0, max_difference = 1.0;
    while (max_difference > 0) {
        for (int i = 0; i < graph.outgoing_edges.size(); i++) {
            int min_index;
            for_each(graph.outgoing_edges[i].begin(), graph.outgoing_edges[i].end(), [&](int &j)-> int {
                int min_path_cost = INT_MAX;
                
                if (discovered_vertex.at(j) == false && graph.vertex_path_cost.at(j) <= min_path_cost) {
                    min_path_cost = graph.vertex_path_cost.at(j);
                    min_index = j;
                }
            });
            
            discovered_vertex.at(min_index) = true;
            
            int temp = graph.vertex_path_cost.at(min_index);
            graph.set_vertex_rank(i, 
                [&](void)->double {
                    for(int j = 0; j < graph.outgoing_edges.size(); j++) {
                        if(!discovered_vertex.at(j) && 
                           graph.vertex_path_cost.at(min_index) != INT_MAX && 
                           graph.vertex_path_cost.at(min_index) + 1 < graph.vertex_path_cost.at(i)) {
                               graph.vertex_path_cost.at(i) = graph.vertex_path_cost.at(min_index) + 1;
                        }
                        
                        difference = abs(temp - graph.vertex_path_cost.at(min_index));
                        if (difference < max_difference) {
                            max_difference = difference;
                        }
                        
                    }
                }
            );
        }
        graph.print_vertex_ranks();
    }
}

int main(int argc, char *argv[]) {
    string dataset = argv[1];
    AdjacencyList graph (dataset);
    
    rank_distribution(graph);
    graph.print_vertex_ranks();
}

