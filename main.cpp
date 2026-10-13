#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include "Graph.h"
#include "RouteOptimizer.h"
#include "Ambulance.h"
#include "Firetruck.h"
#include "policevehicle.h"
using namespace std;

const double MAX_SNAP_DISTANCE = 1000.0; // metres, a point further than this is outside the map

// one possible way of reaching the emergency
struct Candidate
{
    int vehicle;        // index in the fleet
    string type;        // "fastest", "shortest" or both
    vector<int> path;
    double distance;    // metres
    double time;        // seconds
};

// name of the road between two consecutive nodes of the path
string roadNameBetween(const vector<Edge>& edges, const vector<Road>& roads, int a, int b)
{
    Edge e;
    Road r;
    // two way roads are stored only once, so try both directions
    if (!findEdge(edges, a, b, e) && !findEdge(edges, b, a, e))
        return "unnamed road";
    if (!getRoadInfo(roads, e.roadId, r))
        return "unnamed road";
    if (!r.name.empty())
        return r.name;
    if (!r.type.empty())
    {
        string type = r.type;
        for (size_t i = 0; i < type.size(); i++)
        {
            if (type[i] == '_')
                type[i] = ' ';
        }
        return "unnamed " + type + " road";
    }
    return "unnamed road";
}

string toKm(double metres)
{
    ostringstream out;
    out << fixed << setprecision(2) << metres / 1000.0;
    return out.str();
}

string toMinutes(double seconds)
{
    ostringstream out;
    out << fixed << setprecision(1) << seconds / 60.0;
    return out.str();
}

// reads "lat lon" (a comma between them is also accepted)
// returns false when the user wants to quit
bool readLocation(double& lat, double& lon)
{
    string line;
    while (true)
    {
        cout << "\nEnter emergency location as latitude longitude (q to quit): ";
        if (!getline(cin, line))
            return false;
        if (line == "q" || line == "Q" || line == "quit")
            return false;

        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == ',')
                line[i] = ' ';
        }
        istringstream in(line);
        string extra;
        if (!(in >> lat >> lon) || (in >> extra))
        {
            cout << "Invalid input. Example: 29.20 79.52" << endl;
            continue;
        }
        if (lat < -90 || lat > 90 || lon < -180 || lon > 180)
        {
            cout << "Latitude must be -90 to 90 and longitude -180 to 180." << endl;
            continue;
        }
        return true;
    }
}

int main()
{
    vector<Node> nodes;
    vector<Edge> edges;
    vector<Road> roads;
    unordered_map<int, vector<pair<int, double>>> graph;      // weight = metres
    unordered_map<int, vector<pair<int, double>>> timeGraph;  // weight = seconds

    if (!buildRoadNetwork("export.geojson", nodes, edges, roads, graph))
    {
        return 1;
    }
    buildTimeGraph(edges, roads, timeGraph);
    printGraphStats(nodes, edges, graph);

    // starting positions of the vehicles, each one is placed on the nearest road node
    vector<EmergencyVehicle*> fleet;
    fleet.push_back(new Ambulance("AMB-01", "Haldwani Bypass",
                                  findNearestNode(nodes, 29.2680085, 79.5492688)));
    fleet.push_back(new Firetruck("FIRE-01", "Tiwari Colony",
                                  findNearestNode(nodes, 29.2352046, 79.5095438)));
    fleet.push_back(new Policevehicle("POL-01", "Station B",
                                      findNearestNode(nodes, 29.1836007, 79.5821893)));

    cout << "\n------ rapid route fleet ---------" << endl;
    for (size_t i = 0; i < fleet.size(); i++)
    {
        fleet[i]->respondToemergency();
    }

    // same Dijkstra class, run once on the distance graph and once on the time graph
    RouteOptimizer shortestOptimizer(graph, (int)nodes.size());
    RouteOptimizer fastestOptimizer(timeGraph, (int)nodes.size());

    double lat, lon;
    while (readLocation(lat, lon))
    {
        int emergencyNode = findNearestNode(nodes, lat, lon);
        double nodeLat, nodeLon;
        getNodeCoordinates(nodes, emergencyNode, nodeLat, nodeLon);
        double snapDistance = calculateDistance(lat, lon, nodeLat, nodeLon);
        if (snapDistance > MAX_SNAP_DISTANCE)
        {
            cout << "That location is " << toKm(snapDistance)
                 << " km from the nearest road, it is outside the map." << endl;
            continue;
        }

        // collect every route that can reach the emergency
        vector<Candidate> candidates;
        for (size_t i = 0; i < fleet.size(); i++)
        {
            int from = fleet[i]->getcurrentNodeId();
            double cost;

            Candidate fastest;
            fastest.vehicle = (int)i;
            fastest.type = "fastest";
            fastest.path = fastestOptimizer.findShortestPath(from, emergencyNode, cost);
            if (fastest.path.empty())
                continue;
            fastest.time = cost;
            fastest.distance = getPathCost(graph, fastest.path);

            Candidate shortest;
            shortest.vehicle = (int)i;
            shortest.type = "shortest";
            shortest.path = shortestOptimizer.findShortestPath(from, emergencyNode, cost);
            shortest.distance = cost;
            shortest.time = getPathCost(timeGraph, shortest.path);

            candidates.push_back(fastest);
            if (shortest.path == fastest.path)
                candidates.back().type = "fastest+shortest";
            else
                candidates.push_back(shortest);
        }

        if (candidates.empty())
        {
            cout << "No route found." << endl;
            continue;
        }

        // the most appropriate route is the one that reaches the emergency first
        size_t best = 0;
        cout << "\nCandidate routes:" << endl;
        for (size_t i = 0; i < candidates.size(); i++)
        {
            const Candidate& c = candidates[i];
            cout << "  " << i + 1 << ") " << fleet[c.vehicle]->getvehicle_id()
                 << "  " << left << setw(17) << c.type << right
                 << setw(8) << toKm(c.distance) << " km"
                 << setw(8) << toMinutes(c.time) << " min" << endl;
            if (c.time < candidates[best].time ||
                (c.time == candidates[best].time && c.distance < candidates[best].distance))
            {
                best = i;
            }
        }

        const Candidate& chosen = candidates[best];
        EmergencyVehicle* vehicle = fleet[chosen.vehicle];
        cout << "\nSelected route " << best + 1 << ": " << vehicle->getvehicle_id()
             << " (" << chosen.type << ")" << endl;
        cout << "Distance: " << toKm(chosen.distance) << " km, estimated time: "
             << toMinutes(chosen.time) << " min" << endl;
        cout << "Nodes on route: " << chosen.path.size() << "\n" << endl;

        // the vehicle reports each road when it leaves it
        string currentRoad = "";
        double roadLength = 0;
        for (size_t i = 0; i + 1 < chosen.path.size(); i++)
        {
            int a = chosen.path[i];
            int b = chosen.path[i + 1];
            string name = roadNameBetween(edges, roads, a, b);
            if (i == 0)
            {
                currentRoad = name;
            }
            else if (name != currentRoad)
            {
                vehicle->updatelocation(currentRoad + " (" + toKm(roadLength) + " km)");
                currentRoad = name;
                roadLength = 0;
            }
            // node ids are sequential, so nodes[id] is the node with that id
            roadLength += calculateDistance(nodes[a].latitude, nodes[a].longitude,
                                            nodes[b].latitude, nodes[b].longitude);
        }
        if (!currentRoad.empty())
        {
            vehicle->updatelocation(currentRoad + " (" + toKm(roadLength) + " km)");
        }

        // the vehicle stays at the emergency, so the next request starts from here
        vehicle->setcurrentNodeId(emergencyNode);
        ostringstream where;
        where << fixed << setprecision(5) << nodeLat << ", " << nodeLon;
        vehicle->finishtrip(where.str());
    }

    cout << "\nGoodbye." << endl;
    for (size_t i = 0; i < fleet.size(); i++)
    {
        delete fleet[i];
    }
    return 0;
}
