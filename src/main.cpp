#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <mutex>
#include <utility>
#include <deque>
#include <climits>
#include <iostream>
#include <tuple>

#include "adjacencylist.h"

using namespace std;
using namespace std::chrono;

struct partitions {
    int l_bound = 0;
    int r_bound = 0;
    int max_difference = 1;
    short thread_id;
};

vector<bool> converged;
mutex mutex_converged;
deque<mutex> mutex_map_weight;
pthread_barrier_t barrier;

void update_vertex(AdjacencyList& graph, partitions& part, int& difference) {
    
    for (int i = part.l_bound; i <= part.r_bound; i++) {
        for_each(graph.outgoing_edges[i].begin(), graph.outgoing_edges[i].end(), [&](int &j) {
            
            mutex_map_weight.at(j).lock();
            
            // cout << "i, j: " << i << "\t" << j << endl;
            int initial = graph.vertex_weight.at(j);
            
            if (j == 0) {
                mutex_map_weight.at(j).unlock();
                return;
            }
            
            mutex_map_weight.at(i).lock();
            
            if (graph.vertex_weight.at(j) == INT_MAX && graph.vertex_weight.at(i) != INT_MAX) {
                graph.vertex_weight.at(j) = graph.vertex_weight.at(i) + 1;
            }
            else if (graph.vertex_weight.at(j) != INT_MAX && graph.vertex_weight.at(i) + 1 <= initial) {
                graph.vertex_weight.at(j) = graph.vertex_weight.at(i) + 1;
            }
            
            mutex_map_weight.at(i).unlock();
            
            difference = abs(initial - graph.vertex_weight.at(j));
            
            mutex_map_weight.at(j).unlock();
            
            if (difference < part.max_difference) {
                part.max_difference = difference;
            }
        });
    }
}

void shortest_path(AdjacencyList& graph, partitions part) {
    graph.vertex_weight.at(0) = 0;
    
    int difference = 1;
    
    while ((part.max_difference > 0) && 
            !all_of(converged.begin(), converged.end(), [](bool v) { return v; })) {
                
        fill(converged.begin(), converged.end(), false);
                
        update_vertex(graph, part, difference);

        int rc = pthread_barrier_wait (&barrier);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            cout << "Could not wait on barrier.\n";
            exit(-1);
        }
        
        lock_guard<mutex> lock(mutex_converged);
        if (part.max_difference == 0) converged.at(part.thread_id) = true; 
        
        cout << !all_of(converged.begin(), converged.end(), [](bool v) { return v; } ) << endl;        
        
    }
    cout << "Thread " << part.thread_id << " exited.\n" << endl;
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
    converged.resize(num_threads);
    
    pthread_barrier_init (&barrier, NULL, num_threads);
    
    mutex_map_weight.resize(graph.vertex_weight.size());
    
    if(num_threads == 1) {
        thread_distribution.at(0).l_bound = 0;
        thread_distribution.at(0).r_bound = graph.incoming_edges.size() - 1;
        
        cout << "Thread 0 started.\n";
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
            
            // cout << "\n Thread " << i << " started.\n";
            threads[i] = thread(shortest_path, ref(graph), thread_distribution.at(i));
        }
    }
    
    // for(int iter = 0; iter < thread_distribution.size(); iter++) {
    //     cout << "\n Thread Num: " << thread_distribution.at(iter).thread_id
    //          << "\t" << thread_distribution.at(iter).l_bound
    //          << "\t" << thread_distribution.at(iter).r_bound;
    // }
    
    // cout << "\n\n";

    if(num_threads != 1)
        for_each(threads.begin(), threads.end(), mem_fn(&thread::join));
    
    graph.print_vertex_ranks();
    
    return 0;
}