#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

// -------------------------------------------------------
// 1) Clear screen
// -------------------------------------------------------
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// -------------------------------------------------------
// 2) Display a grid
// -------------------------------------------------------
void displayGrid(const vector<vector<char>>& grid, int droneR, int droneC) {
    int rows = grid.size();
    if(rows == 0) return;
    int cols = grid[0].size();

    for (int r = 0; r < rows; r++) {
        cout << "+";
        for (int c = 0; c < cols; c++) {
            cout << "---+";
        }
        cout << "\n";

        for (int c = 0; c < cols; c++) {
            cout << "| ";
            if(r == droneR && c == droneC) {
                cout << "D ";
            } else {
                cout << grid[r][c] << " ";
            }
        }
        cout << "|\n";
    }
    // Bottom boundary
    cout << "+";
    for(int c=0; c<grid[0].size(); c++){
        cout << "---+";
    }
    cout << "\n";
}

// -------------------------------------------------------
// 3) A* data
// -------------------------------------------------------
struct Node {
    int r, c;
    int gCost;
    int hCost;
    int fCost() const { return gCost + hCost; }
};

struct NodeCompare {
    bool operator()(const Node& a, const Node& b) {
        return a.fCost() > b.fCost();
    }
};

// Manhattan distance
int manhattan(int r1, int c1, int r2, int c2){
    return abs(r1 - r2) + abs(c1 - c2);
}

// -------------------------------------------------------
// 4) A* function
// -------------------------------------------------------
vector<pair<int,int>> aStarPath(int rows, int cols,
                                int sr, int sc,
                                int gr, int gc)
{
    static vector<pair<int,int>> directions = {
        {-1,0},{1,0},{0,-1},{0,1}
    };

    vector<vector<int>> gCost(rows, vector<int>(cols, 999999));
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));

    priority_queue<Node, vector<Node>, NodeCompare> openSet;

    gCost[sr][sc] = 0;
    int h0 = manhattan(sr, sc, gr, gc);
    openSet.push(Node{sr, sc, 0, h0});

    while(!openSet.empty()){
        Node cur = openSet.top();
        openSet.pop();
        int r = cur.r, c = cur.c;
        if(closed[r][c]) continue;
        closed[r][c] = true;

        if(r == gr && c == gc){
            // Reconstruct path
            vector<pair<int,int>> path;
            while(!(r == sr && c == sc)){
                path.push_back({r,c});
                auto p = parent[r][c];
                r = p.first; c = p.second;
            }
            path.push_back({sr,sc});
            reverse(path.begin(), path.end());
            return path;
        }

        for(auto &d : directions){
            int nr = r + d.first;
            int nc = c + d.second;
            if(nr<0 || nr>=rows || nc<0 || nc>=cols) continue;
            if(closed[nr][nc]) continue;

            int newG = gCost[r][c] + 1; // cost=1 for each step
            if(newG < gCost[nr][nc]){
                gCost[nr][nc] = newG;
                int h = manhattan(nr,nc,gr,gc);
                parent[nr][nc] = {r,c};
                openSet.push(Node{nr, nc, newG, h});
            }
        }
    }

    return {}; // no path
}

// -------------------------------------------------------
// 5) Reveal a 3x3 zone
// -------------------------------------------------------
void revealAround(const vector<vector<char>>& actualMap,
                  vector<vector<char>>& droneMap,
                  int centerR, int centerC,
                  int detectionRange=1)
{
    int rows = actualMap.size();
    if(rows==0) return;
    int cols = actualMap[0].size();

    for(int dr=-detectionRange; dr<=detectionRange; dr++){
        for(int dc=-detectionRange; dc<=detectionRange; dc++){
            int rr = centerR+dr;
            int cc = centerC+dc;
            if(rr>=0 && rr<rows && cc>=0 && cc<cols){
                // If there's a fire, mark 'X', else ' '
                if(actualMap[rr][cc] == 'X') {
                    droneMap[rr][cc] = 'X';
                } else {
                    // only overwrite '?' or ' ' if not discovered fire
                    if(droneMap[rr][cc] == '?' || droneMap[rr][cc] == ' '){
                        droneMap[rr][cc] = ' ';
                    }
                }
            }
        }
    }
}

// -------------------------------------------------------
// 6) Find all discovered fires in droneMap
// -------------------------------------------------------
vector<pair<int,int>> findFires(const vector<vector<char>>& droneMap) {
    vector<pair<int,int>> fires;
    int rows = droneMap.size();
    if(rows==0) return fires;
    int cols = droneMap[0].size();

    for(int r=0; r<rows; r++){
        for(int c=0; c<cols; c++){
            if(droneMap[r][c] == 'X'){
                fires.push_back({r,c});
            }
        }
    }
    return fires;
}

// -------------------------------------------------------
// 7) Find next unknown '?' in row-major
// -------------------------------------------------------
pair<int,int> findNextUnknown(const vector<vector<char>>& droneMap) {
    int rows = droneMap.size();
    if(rows==0) return {-1,-1};
    int cols = droneMap[0].size();

    for(int r=0; r<rows; r++){
        for(int c=0; c<cols; c++){
            if(droneMap[r][c] == '?'){
                return {r,c};
            }
        }
    }
    return {-1,-1};
}

// -------------------------------------------------------
// 8) Decide next target: discovered fire or unknown cell
//    - We pick the fire that yields the shortest path
//    - If none reachable, we pick the next '?' cell
//    - If no '?' remain, return (-1,-1)
// -------------------------------------------------------
pair<int,int> chooseTarget(const vector<vector<char>>& droneMap,
                           int droneR, int droneC)
{
    // 1) Check for discovered fires
    auto fires = findFires(droneMap);
    if(!fires.empty()){
        // pick the best by path length
        int rows = droneMap.size();
        int cols = droneMap[0].size();

        vector<pair<int,int>> bestPath;
        int bestDist = 999999;
        pair<int,int> bestFire{-1,-1};

        for(auto &f : fires){
            auto path = aStarPath(rows, cols, droneR, droneC, f.first, f.second);
            if(!path.empty()){
                int dist = path.size()-1;
                if(dist < bestDist){
                    bestDist = dist;
                    bestPath = path;
                    bestFire = f;
                }
            }
        }

        if(!bestPath.empty()){
            // We found a reachable fire
            return bestFire;
        }
    }

    // 2) No reachable fires => find next '?' to scan
    return findNextUnknown(droneMap);
}

// -------------------------------------------------------
// 9) Main
// -------------------------------------------------------
int main(){
    srand((unsigned)time(nullptr));

    int rows = 4;
    int cols = 8;

    // actual map
    vector<vector<char>> actualMap(rows, vector<char>(cols,' '));
    for(int r=0; r<rows; r++){
        for(int c=0; c<cols; c++){
            if(rand()%100 < 25) {
                actualMap[r][c] = 'X';
            }
        }
    }

    // drone map: '?' for unknown
    vector<vector<char>> droneMap(rows, vector<char>(cols,'?'));

    // start drone
    int droneR=0, droneC=0;
    // reveal around
    revealAround(actualMap, droneMap, droneR, droneC, 1);

    // discovered fires
    vector<pair<int,int>> discoveredFires;

    // track last target to avoid repeated identical targeting
    pair<int,int> previousTarget {-1, -1};

    while(true){
        clearScreen();
        cout << "Drone scanning...\n";
        displayGrid(droneMap, droneR, droneC);
        this_thread::sleep_for(chrono::milliseconds(300));

        // 1) pick target
        auto target = chooseTarget(droneMap, droneR, droneC);

        // no unknown left or no fire => target = (-1,-1)
        if(target.first == -1 && target.second == -1){
            cout << "No more targets.\n";
            break;
        }

        // if it's the same as previous target, we might be stuck
        if(target == previousTarget){
            cout << "Stuck: same target as previous (" << target.first
                 << "," << target.second << "). Breaking out.\n";
            break;
        }
        previousTarget = target;

        // 2) pathfind
        auto path = aStarPath(rows, cols, droneR, droneC, target.first, target.second);
        if(path.empty()){
            cout << "No path to target. Breaking.\n";
            break;
        }

        // store old position to detect no movement
        int oldR = droneR;
        int oldC = droneC;

        // 3) follow the path (except path[0] which is current location)
        for(size_t i=1; i<path.size(); i++){
            droneR = path[i].first;
            droneC = path[i].second;

            // reveal around
            revealAround(actualMap, droneMap, droneR, droneC, 1);

            // if we step on 'X', mark discovered
            if(droneMap[droneR][droneC] == 'X'){
                discoveredFires.push_back({droneR, droneC});
                // remove it so we don't retarget
                droneMap[droneR][droneC] = ' ';
            }

            // show
            clearScreen();
            displayGrid(droneMap, droneR, droneC);
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        // if we ended up not moving at all, break
        if(droneR == oldR && droneC == oldC){
            cout << "Drone did not move. Possibly stuck. Breaking.\n";
            break;
        }
    }

    // final
    clearScreen();
    cout << "Drone final map:\n";
    displayGrid(droneMap, -1, -1);

    cout << "\nDiscovered fires:\n";
    if(discoveredFires.empty()){
        cout << "None\n";
    } else {
        for(auto &f : discoveredFires){
            cout << "(" << f.first << "," << f.second << ")\n";
        }
    }

    cout << "Done.\n";
    return 0;
}
