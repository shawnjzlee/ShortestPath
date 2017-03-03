#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <mutex>
#include <utility>
#include <deque>
#include <climits>
#include <iostream>

#include "adjacencylist.h"

using namespace std;
using namespace std::chrono;

struct partitions {
    int l_bound = 0;
    int r_bound = 0;
    int max_difference = 0;
    short thread_id;
};

deque<mutex> mutex_map_weight;
mutex mutex_difference;
pthread_barrier_t barrier;

int g_max_difference = 1;

void update_vertex(AdjacencyList& graph, partitions part) {
    int initial, difference = 1;
    part.max_difference = 0, g_max_difference = 0;
    
    for (int i = part.l_bound; i < part.r_bound; i++) {
        for (int j = 0; j < graph.outgoing_edges.at(i).size(); j++) {
            
            mutex_map_weight.at(j).lock();
            initial = graph.vertex_weight.at(j);
            
            if (j == 0) return;
            
            mutex_map_weight.at(i).lock();
            if (graph.vertex_weight.at(j) == INT_MAX && graph.vertex_weight.at(i) != INT_MAX) {
                graph.vertex_weight.at(j) = graph.vertex_weight.at(i) + 1;
                graph.vertex_weight.at(j) = INT_MAX - 1;
            }
            else if (graph.vertex_weight.at(j) != INT_MAX && graph.vertex_weight.at(i) + 1 <= initial) {
                graph.vertex_weight.at(j) = graph.vertex_weight.at(i) + 1;
            }
            mutex_map_weight.at(i).unlock();
            difference = abs(initial - graph.vertex_weight.at(j));
            mutex_map_weight.at(j).unlock();
            if (difference > part.max_difference) {
                part.max_difference = difference;
            }
        }
    }
    lock_guard<mutex> lock(mutex_difference);
    if (part.max_difference > g_max_difference) {
        g_max_difference = part.max_difference;
    }
}

void shortest_path(AdjacencyList& graph, partitions part) {
    graph.vertex_weight.at(0) = 0;
    
    do {
        update_vertex(graph, part);
        
        int rc = pthread_barrier_wait (&barrier);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            cout << "Could not wait on barrier.\n";
            exit(-1);
        }
    } while (g_max_difference > 0);
    
}


int main(int argc, char * argv[]) {
    if (argc != 3) {
        cout << "Not enough arguments.\n"
             << "Requires [dataset] [number of threads].\n\n"
             << "Using `make check`: make check ARGS1=\"../../dataset/[file_name]\" ARGS2=\"[number of threads]\"\n"
             << "Command-line: ./sssp ../../dataset/[file_name] [number of threads]\n";
        return -1;
    }
    
    string dataset = argv[1];
    AdjacencyList graph(dataset);
    
    int num_threads = 0, remaining_edges = 0;
    
    if (atoi(argv[2]) == 0) num_threads = 1;
    else if (atoi(argv[2]) > thread::hardware_concurrency()) {
        cout << "num_threads set to maximum supported concurrent threads"
             << " (" << thread::hardware_concurrency() << ")\n";
        num_threads = thread::hardware_concurrency() - 1;
    }
    else num_threads = atoi(argv[2]);
    
    if (graph.incoming_edges.size() % num_threads)
        remaining_edges = graph.incoming_edges.size() % num_threads;
        
    int num_edges = graph.incoming_edges.size() / num_threads;
    
    vector<partitions> thread_distribution(num_threads);
    vector<thread> threads(num_threads);
    pthread_barrier_init (&barrier, NULL, num_threads);
    
    mutex_map_weight.resize(graph.vertex_weight.size());
    
    if(num_threads == 0) {
        thread_distribution.at(0).r_bound = graph.incoming_edges.size() - 1;
        
        shortest_path(graph, thread_distribution.at(0));
        graph.print_vertex_ranks();
        return 0;
    }
    else {
        for (int i = 0, edges_distributed = 0; i < num_threads; i++, edges_distributed++) {
            thread_distribution[i].thread_id = i;
            thread_distribution[i].l_bound = edges_distributed;
            if (remaining_edges != 0) {
                thread_distribution[i].r_bound = edges_distributed + num_edges;
                edges_distributed += num_edges;
                remaining_edges--;
            }
            else {
                thread_distribution[i].r_bound = edges_distributed + num_edges - 1;
                edges_distributed += num_edges - 1;
            }
            
            cout << "\n Thread " << i << " started.\n";
            threads[i] = thread(shortest_path, ref(graph), thread_distribution.at(i));
        }
    }
    
    for(int iter = 0; iter < thread_distribution.size(); iter++) {
        cout << "\n Thread Num: " << thread_distribution.at(iter).thread_id
             << "\t" << thread_distribution.at(iter).l_bound
             << "\t" << thread_distribution.at(iter).r_bound;
    }
    
    cout << "\n\n";
    
    if(num_threads != 1)
        for_each(threads.begin(), threads.end(), mem_fn(&thread::join));
    
    graph.print_vertex_ranks();
    
    return 0;
}