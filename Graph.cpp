#define _USE_MATH_DEFINES
#include "Graph.h"
#include "json.hpp"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_set>
using json = nlohmann::json;
double calculateDistance(double lat1, double lon1,
                         double lat2, double lon2)
{
    const double R = 6371000.0;
    double lat1Rad = lat1 * M_PI / 180.0;
    double lat2Rad = lat2 * M_PI / 180.0;
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1Rad) * cos(lat2Rad) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}
string makeKey(double latitude, double longitude)
{
    return to_string(latitude) + "," + to_string(longitude);
}
void loadGraph(
    vector<Node>& nodes,
    vector<Edge>& edges,
    unordered_map<int, vector<pair<int, double>>>& graph)
{
    ifstream graphFile("graph.txt");
    if (!graphFile)
    {
        cout << "Graph file could not be opened." << endl;
        return;
    }
    char type;
    while (graphFile >> type)
    {
        if (type == 'N')
        {
            Node node;

            graphFile >> node.id
                  >> node.latitude
                  >> node.longitude;

            nodes.push_back(node);
        }
        else if (type == 'E')
        {
            Edge edge;
            graphFile >> edge.from>> edge.to>> edge.distance>> edge.oneway>> edge.roadId;
            edges.push_back(edge);
        }
    }
    graphFile.close();
    buildGraph(edges, graph);
    cout << "Graph loaded successfully!" << endl;
    cout << "Nodes loaded: " << nodes.size() << endl;
    cout << "Edges loaded: " << edges.size() << endl;
}
void saveGraph(
    const vector<Node>& nodes,
    const vector<Edge>& edges)
{
    ofstream graphFile("graph.txt");
    if (!graphFile)
    {
        cout << "Graph file could not be opened." << endl;
        return;
    }
    graphFile << fixed << setprecision(7);
    for (const auto& node : nodes)
    {
        graphFile << "N "
                  << node.id << " "
                  << node.latitude << " "
                  << node.longitude << "\n";
    }
    for (const auto& edge : edges)
    {
        graphFile << "E "
          << edge.from << " "
          << edge.to << " "
          << edge.distance << " "
          << edge.oneway << " "
          << edge.roadId << "\n";
    }
    graphFile.close();
    cout << "Graph saved successfully!" << endl;
}
// adds one edge to the adjacency list, the direction depends on the oneway value
static void addToGraph(
    unordered_map<int, vector<pair<int, double>>>& graph,
    const Edge& edge,
    double weight)
{
    if (edge.oneway == "yes")
    {
        graph[edge.from].push_back({edge.to, weight});
    }
    else if (edge.oneway == "no")
    {
        graph[edge.from].push_back({edge.to, weight});
        graph[edge.to].push_back({edge.from, weight});
    }
    else if (edge.oneway == "-1")
    {
        graph[edge.to].push_back({edge.from, weight});
    }
}
// weight of every edge is its length in metres
void buildGraph(
    const vector<Edge>& edges,
    unordered_map<int, vector<pair<int, double>>>& graph)
{
    for (const auto& edge : edges)
    {
        addToGraph(graph, edge, edge.distance);
    }
}
// roads a vehicle cannot drive on
bool isDrivable(const string& highwayType)
{
    return !(highwayType == "path" || highwayType == "footway" ||
             highwayType == "pedestrian" || highwayType == "steps" ||
             highwayType == "cycleway" || highwayType == "bridleway" ||
             highwayType == "corridor" || highwayType == "construction" ||
             highwayType == "proposed" || highwayType == "platform");
}
// assumed average speed (km/h) of an emergency vehicle on each road type
double getRoadSpeed(const string& highwayType)
{
    if (highwayType == "motorway") return 80;
    if (highwayType == "trunk") return 60;
    if (highwayType == "primary") return 50;
    if (highwayType == "secondary") return 45;
    if (highwayType == "tertiary") return 40;
    if (highwayType == "motorway_link" || highwayType == "trunk_link") return 40;
    if (highwayType == "primary_link" || highwayType == "secondary_link" ||
        highwayType == "tertiary_link") return 30;
    if (highwayType == "unclassified") return 35;
    if (highwayType == "residential") return 30;
    if (highwayType == "service") return 20;
    if (highwayType == "living_street" || highwayType == "track") return 15;
    return 25;
}
// weight of every edge is the travel time in seconds (length / road speed)
void buildTimeGraph(
    const vector<Edge>& edges,
    const vector<Road>& roads,
    unordered_map<int, vector<pair<int, double>>>& graph)
{
    for (const auto& edge : edges)
    {
        double speed = 25;
        if (edge.roadId >= 0 && edge.roadId < (int)roads.size())
        {
            speed = getRoadSpeed(roads[edge.roadId].type);
        }
        double seconds = edge.distance / (speed / 3.6);
        addToGraph(graph, edge, seconds);
    }
}
// total weight of a path in the given graph, -1 if two nodes of the path are not connected
double getPathCost(
    const unordered_map<int, vector<pair<int, double>>>& graph,
    const vector<int>& path)
{
    double total = 0;
    for (size_t i = 0; i + 1 < path.size(); i++)
    {
        auto it = graph.find(path[i]);
        if (it == graph.end())
            return -1;

        double best = 1e18;
        for (const auto& neighbour : it->second)
        {
            if (neighbour.first == path[i + 1] && neighbour.second < best)
                best = neighbour.second;
        }
        if (best >= 1e18)
            return -1;
        total += best;
    }
    return total;
}
int countIntersections(
    const unordered_map<int, vector<pair<int, double>>>& graph)
{
    int count = 0;

    for (auto& connection : graph)
    {
        if (connection.second.size() >= 3)
        {
            count++;
        }
    }

    return count;
}
int countIsolatedNodes(
    const vector<Node>& nodes,
    const unordered_map<int, vector<pair<int, double>>>& graph)
{
    int count = 0;

    for (auto& node : nodes)
    {
        if (graph.find(node.id) == graph.end())
        {
            count++;
        }
    }

    return count;
}
int countConnectedComponents(
    const vector<Node>& nodes,
    const unordered_map<int, vector<pair<int, double>>>& graph)
{
    unordered_set<int> visited;
    int components = 0;

    for (auto& node : nodes)
    {
        if (visited.find(node.id) != visited.end())
            continue;

        components++;

        queue<int> q;
        q.push(node.id);
        visited.insert(node.id);

        while (!q.empty())
        {
            int current = q.front();
            q.pop();

            if (graph.find(current) == graph.end())
                continue;

            for (auto& neighbour : graph.at(current))
            {
                int next = neighbour.first;

                if (visited.find(next) == visited.end())
                {
                    visited.insert(next);
                    q.push(next);
                }
            }
        }
    }
    return components;
}
int countEndpointConnections(
    const vector<Edge>& edges)
{
    unordered_map<int, int> connectionCount;

    for (auto& edge : edges)
    {
        connectionCount[edge.from]++;
        connectionCount[edge.to]++;
    }

    int count = 0;

    for (auto& connection : connectionCount)
    {
        if (connection.second >= 3)
        {
            count++;
        }
    }

    return count;
}
void printGraphStats(
    const vector<Node>& nodes,
    const vector<Edge>& edges,
    const unordered_map<int, vector<pair<int, double>>>& graph)
{
    int isolatedNodes = 0;
    int intersections = 0;
    for (auto& node : nodes)
    {
        auto it = graph.find(node.id);

        if (it == graph.end())
        {
            isolatedNodes++;
        }
        else if (it->second.size() >= 3)
        {
            intersections++;
        }
    }
    cout << "\n===== GRAPH STATISTICS =====" << endl;
    cout << "Nodes: " << nodes.size() << endl;
    cout << "Edges: " << edges.size() << endl;
    cout << "Intersections: " << intersections << endl;
    cout << "Isolated nodes: " << isolatedNodes << endl;
    cout << "============================" << endl;
}
bool findNodeById(
    const vector<Node>& nodes,
    int id,
    Node& result)
{
    for (auto& node : nodes)
    {
        if (node.id == id)
        {
            result = node;
            return true;
        }
    }

    return false;
}
vector<pair<int, double>> getNeighbors(
    const unordered_map<int, vector<pair<int, double>>>& graph,
    int nodeId)
{
    auto it = graph.find(nodeId);

    if (it != graph.end())
    {
        return it->second;
    }

    return {};
}
bool getNodeCoordinates(
    const vector<Node>& nodes,
    int nodeId,
    double& latitude,
    double& longitude)
{
    for (auto& node : nodes)
    {
        if (node.id == nodeId)
        {
            latitude = node.latitude;
            longitude = node.longitude;
            return true;
        }
    }
    return false;
}
bool findEdge(
    const vector<Edge>& edges,
    int from,
    int to,
    Edge& result)
{
    for (auto& edge : edges)
    {
        if (edge.from == from && edge.to == to)
        {
            result = edge;
            return true;
        }
    }
    return false;
}
bool getRoadInfo(
    const vector<Road>& roads,
    int roadId,
    Road& result)
{
    if (roadId >= 0 && roadId < (int)roads.size())
    {
        result = roads[roadId];
        return true;
    }
    return false;
}
bool buildRoadNetwork(
    const string& filename,
    vector<Node>& nodes,
    vector<Edge>& edges,
    vector<Road>& roads,
    unordered_map<int, vector<pair<int, double>>>& graph)
{
    ifstream file(filename);
    if (!file)
    {
        cout << "GeoJSON file could not be opened." << endl;
        return false;
    }
    json data;
    file >> data;
    cout << "File loaded successfully" << endl;
    cout << "Total features: "
         << data["features"].size() << endl;
    unordered_map<string, int> nodeMap;
    int nextNodeId = 0;
    for (size_t r = 0; r < data["features"].size(); r++)
    {
        const auto& road = data["features"][r];
        if (!road.contains("geometry") || road["geometry"].is_null() || !road.contains("properties"))
        {
            continue;
        }
        string highway = "";
        if (road["properties"].contains("highway") && road["properties"]["highway"].is_string())
        {
            highway = road["properties"]["highway"];
        }
        if (!isDrivable(highway))
        {
            continue;
        }
        string geometryType = road["geometry"]["type"];
        if (geometryType != "LineString")
        {
            continue;
        }
        string oneway = "no";
        if (road["properties"].contains("oneway") && road["properties"]["oneway"].is_string())
        {
            oneway = road["properties"]["oneway"];
        }
        if (oneway == "true" || oneway == "1")
        {
            oneway = "yes";
        }
        else if (oneway != "yes" && oneway != "-1")
        {
            oneway = "no";
        }
        if (road["properties"].contains("junction"))
        {
            if (road["properties"]["junction"] == "roundabout")
            {
                oneway = "yes";
            }
        }
        int currentRoadId = (int)roads.size();
        Road currentRoad;
        if (road["properties"].contains("name") && road["properties"]["name"].is_string())
        {
            currentRoad.name = road["properties"]["name"];
        }
        currentRoad.type = highway;
        currentRoad.oneway = oneway;
        roads.push_back(currentRoad);
        const auto& coordinates = road["geometry"]["coordinates"];
        int previousNodeId = -1;
        for (const auto& point : coordinates)
        {
            double longitude = point[0];
            double latitude = point[1];
            string key = makeKey(latitude, longitude);

            int existingId = -1;
            if (nodeMap.find(key) != nodeMap.end())
            {
                existingId = nodeMap[key];
            }
            int currentNodeId;
            if (existingId == -1)
            {
                Node node;
                node.id = nextNodeId;
                node.latitude = latitude;
                node.longitude = longitude;
                nodes.push_back(node);
                nodeMap[key] = nextNodeId;
                currentNodeId = nextNodeId;
                nextNodeId++;
            }
            else
            {
                currentNodeId = existingId;
            }
            if (previousNodeId != -1)
            {
                Edge edge;
                edge.from = previousNodeId;
                edge.to = currentNodeId;
                edge.distance = calculateDistance(
                    nodes[previousNodeId].latitude,
                    nodes[previousNodeId].longitude,
                    nodes[currentNodeId].latitude,
                    nodes[currentNodeId].longitude
                );
                edge.oneway = oneway;
                edge.roadId = currentRoadId;
                edges.push_back(edge);
            }
            previousNodeId = currentNodeId;
        }
    }
    buildGraph(edges, graph);
    return true;
}
bool nodeExists(
    const vector<Node>& nodes,
    int nodeId)
{
    for (auto& node : nodes)
    {
        if (node.id == nodeId)
        {
            return true;
        }
    }
    return false;
}

int findNearestNode(
    const vector<Node>& nodes,
    double lat,
    double lon)
{
    int bestId = -1;
    double bestDist = 1e18;
    for (auto& node : nodes)
    {
        double d = calculateDistance(lat, lon, node.latitude, node.longitude);
        if (d < bestDist)
        {
            bestDist = d;
            bestId = node.id;
        }
    }
    return bestId;
}
