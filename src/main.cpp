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
#include "global.h"

using namespace std;
using namespace std::chrono;

struct partitions {
    int l_bound;
    int r_bound;
    short thread_id;
};

map<int, unique_ptr<mutex>> mutex_map;

void rank_distribution(AdjacencyList& graph, vector<partitions> thread_distribution) {
    graph.set_vertex_rank(0,0);
    vector <bool> discovered_vertex (graph.vertex_path_cost.size(), false);
    int difference = 1, max_difference = 1;
    while (max_difference > 0) {
        max_difference = 0;
        for (int i = 0; i < graph.outgoing_edges.size(); i++) {
            for_each(graph.outgoing_edges[i].begin(), graph.outgoing_edges[i].end(), [&](int &j) {
                int curr_path_cost = graph.vertex_path_cost.at(j);
                
                if (j == 0) return;
                
                if (discovered_vertex.at(j) == false && graph.vertex_path_cost.at(i) != INT_MAX) {
                    graph.set_vertex_rank(j, graph.vertex_path_cost.at(i) + 1);
                    discovered_vertex.at(j) = true;
                }
                else if (discovered_vertex.at(j) == true && graph.vertex_path_cost.at(i) + 1 <= curr_path_cost) {
                    graph.set_vertex_rank(j, graph.vertex_path_cost.at(i) + 1);
                }
                
                difference = abs(curr_path_cost - graph.vertex_path_cost.at(j));
                if (difference > max_difference) {
                    max_difference = difference;
                }
            });
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cout << "Not enough arguments.\n Requires [dataset] "
             << "[number of threads].\n";
        return -1;
    }
    
    string dataset = argv[1];
    AdjacencyList graph(dataset);
    
    int num_threads = 0, remaining_edges = 0;
    
    if(atoi(argv[2]) == 0) num_threads = 1;
    else if(atoi(argv[2]) > thread::hardware_concurrency()) {
        cout << "num_threads set to maximum supported concurrent threads"
             << " (" << thread::hardware_concurrency() << ") \n";
        num_threads = thread::hardware_concurrency() - 1;
    }
    else num_threads = atoi(argv[2]);
    
    if (graph.incoming_edges.size() % num_threads)
        remaining_edges = graph.incoming_edges.size() % num_threads;
    
    int num_edges = graph.incoming_edges.size() / num_threads;
    
    vector<partitions> thread_distribution(num_threads);
    vector<thread> threads(num_threads);
    
    if (num_threads == 1) {
        thread_distribution.at(0).thread_id = 0;
        thread_distribution.at(0).l_bound = 0;
        thread_distribution.at(0).r_bound = graph.incoming_edges.size() - 1;
        mutex_map.emplace(0, make_unique<mutex>()).first;
        
        rank_distribution(graph, thread_distribution);
        graph.print_vertex_ranks();
        return 0;
    }
    else {
        for (int i = 0, edges_dist = 0; i < num_threads; i++, edges_dist++) {
            thread_distribution[i].thread_id = i;
            thread_distribution[i].l_bound = edges_dist;
            if (remaining_edges != 0) {
                thread_distribution[i].r_bound = edges_dist + num_edges;
                edges_dist += num_edges;
                remaining_edges--;
            }
            else {
                thread_distribution[i].r_bound = edges_dist + num_edges - 1;
                edges_dist += num_edges - 1;
            }
            
            mutex_map.emplace(i, make_unique<mutex>()).first;
            threads[i] = thread(rank_distribution, ref(graph), thread_distribution);
        }
    }
    
    // for(int iter = 0; iter < thread_distribution.size(); iter++) {
    //     cout << "\n Thread Num: " << thread_distribution.at(iter).thread_id
    //          << "\t" << thread_distribution.at(iter).l_bound
    //          << "\t" << thread_distribution.at(iter).r_bound;
    // }
    
    if(num_threads != 1)
        for_each(threads.begin(), threads.end(), mem_fn(&thread::join));
    
    graph.print_vertex_ranks();
    
    return 0;
}