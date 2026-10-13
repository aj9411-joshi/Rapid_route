#ifndef ROUTEOPTIMIZER_H
#define ROUTEOPTIMIZER_H

#include<vector>
#include<utility>
#include<unordered_map>

using namespace std;

class RouteOptimizer{
    private:
    // reference, so the big graph is not copied (it must outlive the optimizer)
    const unordered_map<int, vector<pair<int, double>>>& graph;
    int nodeCount;

    public:

    RouteOptimizer(const unordered_map<int, vector<pair<int, double>>>& g, int n);

    // returns the node ids from start to destination (empty if no route)
    // totalCost is the sum of the edge weights of the graph (metres or seconds), -1 if no route
    vector<int> findShortestPath(int start, int destination, double& totalCost);

};

#endif
