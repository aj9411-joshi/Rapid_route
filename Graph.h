#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;
struct Node
{
    int id;
    double latitude;
    double longitude;
};
struct Edge
{
    int from;
    int to;
    double distance;
    string oneway;
    int roadId;
};
struct Road
{
    string name;
    string type;
    string oneway;
};
double calculateDistance(double lat1, double lon1,
                         double lat2, double lon2);

string makeKey(double latitude, double longitude);
void loadGraph(
    vector<Node>& nodes,
    vector<Edge>& edges,
    unordered_map<int, vector<pair<int, double>>>& graph);
void saveGraph(
    const vector<Node>& nodes,
    const vector<Edge>& edges); 
void buildGraph(
    const vector<Edge>& edges,
    unordered_map<int, vector<pair<int, double>>>& graph);
void buildTimeGraph(
    const vector<Edge>& edges,
    const vector<Road>& roads,
    unordered_map<int, vector<pair<int, double>>>& graph);
bool isDrivable(const string& highwayType);
double getRoadSpeed(const string& highwayType);
double getPathCost(
    const unordered_map<int, vector<pair<int, double>>>& graph,
    const vector<int>& path);
int countIntersections(
    const unordered_map<int, vector<pair<int, double>>>& graph);
int countIsolatedNodes(
    const vector<Node>& nodes,
    const unordered_map<int, vector<pair<int, double>>>& graph);      
int countConnectedComponents(
    const vector<Node>& nodes,
    const unordered_map<int, vector<pair<int, double>>>& graph);
int countEndpointConnections(
    const vector<Edge>& edges);
void printGraphStats(
    const vector<Node>& nodes,
    const vector<Edge>& edges,
    const unordered_map<int, vector<pair<int, double>>>& graph);
bool findNodeById(
    const vector<Node>& nodes,
    int id,
    Node& result);
vector<pair<int, double>> getNeighbors(
    const unordered_map<int, vector<pair<int, double>>>& graph,
    int nodeId);
bool getNodeCoordinates(
    const vector<Node>& nodes,
    int nodeId,
    double& latitude,
    double& longitude);
bool findEdge(
    const vector<Edge>& edges,
    int from,
    int to,
    Edge& result);
bool getRoadInfo(
    const vector<Road>& roads,
    int roadId,
    Road& result);
bool buildRoadNetwork(
    const string& filename,
    vector<Node>& nodes,
    vector<Edge>& edges,
    vector<Road>& roads,
    unordered_map<int, vector<pair<int, double>>>& graph);
bool nodeExists(
    const vector<Node>& nodes,
    int nodeId);
int findNearestNode(
    const vector<Node>& nodes,
    double lat,
    double lon);
#endif