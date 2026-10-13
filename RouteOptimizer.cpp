#include "RouteOptimizer.h"
#include<iostream>
#include<queue>
#include<functional>
#include<algorithm>

RouteOptimizer::RouteOptimizer(const unordered_map<int, vector<pair<int, double>>>& g, int n)
    : graph(g), nodeCount(n)
{
}

vector<int> RouteOptimizer::findShortestPath(int start, int destination, double& totalCost)
{
    const double INF = 1e18;

    // reject node ids that are not on the map
    if (start < 0 || start >= nodeCount || destination < 0 || destination >= nodeCount)
    {
        cout << "Invalid start or destination node." << endl;
        totalCost = -1;
        return {};
    }

    vector<double> distance(nodeCount, INF);
    vector<int> parent(nodeCount, -1);

    distance[start] = 0;

    priority_queue<
        pair<double, int>,
        vector<pair<double, int>>,
        greater<pair<double, int>>
    > pq;

    pq.push({0, start});

    while (!pq.empty())
    {
        double currentDistance = pq.top().first;
        int currentNode = pq.top().second;
        pq.pop();

        // skip old queue entries, a shorter way to this node was already found
        if (currentDistance > distance[currentNode])
            continue;

        // stop early, the shortest distance to destination is final now
        if (currentNode == destination)
            break;

        auto it = graph.find(currentNode);
        if (it == graph.end())
            continue;

        for (auto& road : it->second)
        {
            int nextNode = road.first;
            double roadDistance = road.second;
            double newDistance = currentDistance + roadDistance;

            if (newDistance < distance[nextNode])
            {
                distance[nextNode] = newDistance;
                parent[nextNode] = currentNode;
                pq.push({newDistance, nextNode});
            }
        }
    }

    if (distance[destination] >= INF)
    {
        totalCost = -1;
        return {};
    }

    totalCost = distance[destination];

    vector<int> path;
    int current = destination;

    while (current != -1)
    {
        path.push_back(current);
        current = parent[current];
    }

    reverse(path.begin(), path.end());
    return path;
}
