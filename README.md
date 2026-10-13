# RapidRoute

Command line tool that finds the fastest road route for an emergency vehicle.
The road network comes from an OpenStreetMap export of Haldwani (`export.geojson`).

## How it works
1. `buildRoadNetwork` reads the GeoJSON and builds nodes, edges and an adjacency list.
   Roads a vehicle cannot use (footways, paths, steps, pedestrian streets) are skipped.
2. Two graphs are built from the same edges: one weighted by distance (metres) and one
   weighted by travel time (seconds, length divided by an assumed speed for the road type).
3. The user types the emergency location as `latitude longitude`.
4. `findNearestNode` snaps the location to the closest road node.
5. `RouteOptimizer` (Dijkstra) runs from every vehicle on both graphs, giving a fastest and a
   shortest route per vehicle.
6. All candidate routes are listed and the one with the lowest travel time is selected.

## Build
Keep `json.hpp` and `export.geojson` in the same folder, then:

    g++ -std=c++14 main.cpp Graph.cpp RouteOptimizer.cpp -o RapidRoute

## Run
    ./RapidRoute        (Windows: .\RapidRoute)

Example input: `29.20 79.52`. Type `q` to quit.

## Assumptions
Speeds per road type are fixed averages (see `getRoadSpeed` in `Graph.cpp`), there is no live traffic.
