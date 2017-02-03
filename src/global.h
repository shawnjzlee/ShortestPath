#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <memory>
#include <mutex>

extern std::map<int, std::unique_ptr<std::mutex>> mutex_map_edge;
extern std::map<int, std::unique_ptr<std::mutex>> mutex_map_vertex;
extern int difference;
extern int max_difference;

#endif /* global.h */