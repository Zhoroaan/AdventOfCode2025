#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include <numeric>
#include <thread>
#include <core/Solver.h> // glucose

namespace std
{
    class thread;
}

using namespace std;
const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";


/*
Exact Cover via Dancing Links (DLX) for polyomino tiling.

- Board: W x H flattened to W*H columns (primary constraints: each cell covered exactly once).
- Pieces: for each required copy of each piece type, add one piece-usage column (primary constraint).
- Rows: every possible placement of a piece in some orientation at some board offset;
        the row covers the board cells occupied + exactly one piece-usage column (for that type).

If a solution exists, we print a tiling with labeled placements.
*/


struct ConstraintToEvaluate
{
    std::vector<int> CountTargets;
    int W;
    int H;
};

struct Shape {
    // list of (x,y) coordinates where '#' occurs, relative to (0,0)
    vector<pair<int,int>> cells;
    int w=0, h=0; // bounding box of the original stencil
    string name;  // e.g., "#0"
};

// Utility: rotate points 90 deg clockwise around origin
vector<pair<int,int>> rotate90(const vector<pair<int,int>>& pts) {
    vector<pair<int,int>> r;
    r.reserve(pts.size());
    for (auto [x,y] : pts) r.emplace_back(y, -x);
    return r;
}

// Utility: reflect horizontally (x -> -x)
vector<pair<int,int>> reflectX(const vector<pair<int,int>>& pts) {
    vector<pair<int,int>> r;
    r.reserve(pts.size());
    for (auto [x,y] : pts) r.emplace_back(-x, y);
    return r;
}

// Normalize: shift so min x,y becomes 0,0 and compute width/height
struct OrientedShape {
    vector<pair<int,int>> cells;
    int w=0, h=0;
    string tag; // orientation tag
};

OrientedShape normalize(const vector<pair<int,int>>& pts, const string& tag) {
    int minx=INT_MAX, miny=INT_MAX, maxx=INT_MIN, maxy=INT_MIN;
    for (auto [x,y] : pts) {
        minx=min(minx,x); miny=min(miny,y);
        maxx=max(maxx,x); maxy=max(maxy,y);
    }
    OrientedShape os;
    os.tag = tag;
    for (auto [x,y] : pts) os.cells.emplace_back(x-minx, y-miny);
    os.w = maxx - minx + 1;
    os.h = maxy - miny + 1;
    // sort and unique cells for canonical form
    sort(os.cells.begin(), os.cells.end());
    os.cells.erase(unique(os.cells.begin(), os.cells.end()), os.cells.end());
    return os;
}
vector<OrientedShape> generate_orientations(const Shape& s, bool allowReflections=false) {
    vector<vector<pair<int,int>>> variants;
    auto add_variant = [&](const vector<pair<int,int>>& pts) {
        variants.push_back(pts);
        if (allowReflections) variants.push_back(reflectX(pts));
    };

    vector<pair<int,int>> r0 = s.cells;
    vector<pair<int,int>> r90 = rotate90(r0);
    vector<pair<int,int>> r180 = rotate90(r90);
    vector<pair<int,int>> r270 = rotate90(r180);

    add_variant(r0);
    add_variant(r90);
    add_variant(r180);
    add_variant(r270);

    vector<OrientedShape> uniq;
    unordered_set<string> seen;
    int idx=0;
    for (auto& v : variants) {
        auto os = normalize(v, "o"+to_string(idx++));
        string k;
        for (auto [x,y] : os.cells) k += to_string(x)+","+to_string(y)+";";
        if (!seen.count(k)) { seen.insert(k); uniq.push_back(os); }
    }
    return uniq;
}

Shape make_shape_from_ascii(const vector<string>& rows, const string& name) {
    Shape s;
    s.name = name;
    int h = (int)rows.size();
    int w = 0;
    for (auto& row : rows) w = max(w, (int)row.size());
    s.w = w; s.h = h;
    for (int y=0; y<h; ++y) {
        for (int x=0; x<(int)rows[y].size(); ++x) {
            if (rows[y][x] == '#') s.cells.emplace_back(x,y);
        }
    }
    return s;
}

// ---------------- DLX Implementation ----------------
struct DLX {
    struct Node { int row=-1, col=-1, up, down, left, right; };
    int nCols, nPrimary;           // first nPrimary columns are primary
    vector<int> sz;
    vector<Node> nodes;
    int header;                    // ring of primary column headers only
    vector<int> colHead;

    vector<int> solutionRows;

    DLX(int cols, int primaryCols) : nCols(cols), nPrimary(primaryCols) {
        nodes.reserve(1000000);
        header = newNode();
        nodes[header].left = nodes[header].right = header;
        sz.assign(nCols, 0);
        colHead.assign(nCols, -1);

        int last = header;
        for (int c = 0; c < nCols; ++c) {
            int h = newNode();
            colHead[c] = h;
            nodes[h].col = c;
            nodes[h].up = nodes[h].down = h;
            if (c < nPrimary) {
                // Link primary column headers into the horizontal ring
                nodes[h].left = last;
                nodes[h].right = nodes[last].right;
                nodes[last].right = h;
                nodes[nodes[h].right].left = h;
                last = h;
            } else {
                // Secondary: do NOT link into the primary header ring
                nodes[h].left = nodes[h].right = h;
            }
        }
    }

    int newNode() {
        nodes.push_back(Node{});
        int id = (int)nodes.size() - 1;
        nodes[id].up = nodes[id].down = nodes[id].left = nodes[id].right = id;
        return id;
    }

    void addRow(int rowId, const vector<int>& cols) {
        if (cols.empty()) return;
        int first = -1, prev = -1;
        for (int c : cols) {
            int colHeader = colHead[c];
            int nd = newNode();
            nodes[nd].row = rowId;
            nodes[nd].col = c;
            // insert vertically
            nodes[nd].down = colHeader;
            nodes[nd].up = nodes[colHeader].up;
            nodes[nodes[colHeader].up].down = nd;
            nodes[colHeader].up = nd;
            sz[c]++;

            // link horizontally (circular within the row)
            if (first == -1) first = nd;
            nodes[nd].left = (prev == -1 ? nd : prev);
            nodes[nd].right = (prev == -1 ? nd : nodes[prev].right);
            if (prev != -1) {
                nodes[prev].right = nd;
                nodes[nodes[nd].right].left = nd;
            }
            prev = nd;
        }
        if (first != -1 && prev != -1) {
            nodes[first].left = prev;
            nodes[prev].right = first;
        }
    }

    void cover(int cHeader) {
        // If primary, unlink from the primary header ring
        if (nodes[cHeader].left != cHeader || nodes[cHeader].right != cHeader) {
            nodes[nodes[cHeader].right].left = nodes[cHeader].left;
            nodes[nodes[cHeader].left].right = nodes[cHeader].right;
        }
        // Remove rows that include this column
        for (int r = nodes[cHeader].down; r != cHeader; r = nodes[r].down) {
            for (int j = nodes[r].right; j != r; j = nodes[j].right) {
                int h = colHead[nodes[j].col];
                nodes[nodes[j].down].up = nodes[j].up;
                nodes[nodes[j].up].down = nodes[j].down;
                sz[nodes[j].col]--;
            }
        }
    }

    void uncover(int cHeader) {
        // Restore rows
        for (int r = nodes[cHeader].up; r != cHeader; r = nodes[r].up) {
            for (int j = nodes[r].left; j != r; j = nodes[j].left) {
                int h = colHead[nodes[j].col];
                nodes[nodes[j].down].up = j;
                nodes[nodes[j].up].down = j;
                sz[nodes[j].col]++;
            }
        }
        // If primary, relink into primary header ring
        if (nodes[cHeader].left != cHeader || nodes[cHeader].right != cHeader) {
            nodes[nodes[cHeader].right].left = cHeader;
            nodes[nodes[cHeader].left].right = cHeader;
        }
    }

    int chooseColumn() {
        int best = -1, minsz = INT_MAX;
        // iterate primary headers via the header ring
        for (int h = nodes[header].right; h != header; h = nodes[h].right) {
            int c = nodes[h].col;
            if (sz[c] < minsz) { minsz = sz[c]; best = h; }
        }
        return best;
    }

    bool search() {
        // solved when there are no primary columns left
        if (nodes[header].right == header) return true;

        int cHeader = chooseColumn();
        if (cHeader == -1) return false;
        if (sz[nodes[cHeader].col] == 0) return false;

        cover(cHeader);
        for (int r = nodes[cHeader].down; r != cHeader; r = nodes[r].down) {
            solutionRows.push_back(nodes[r].row);
            // cover other columns in this row (both primary and secondary)
            for (int j = nodes[r].right; j != r; j = nodes[j].right) {
                int h = colHead[nodes[j].col];
                cover(h);
            }
            if (search()) return true;
            // backtrack
            for (int j = nodes[r].left; j != r; j = nodes[j].left) {
                int h = colHead[nodes[j].col];
                uncover(h);
            }
            solutionRows.pop_back();
        }
        uncover(cHeader);
        return false;
    }

    bool solveOne() { return search(); }
};
inline int cellIndex(int x, int y, int W) { return y * W + x; }
// Sequential counter encoding for "at most one"
void addAtMostOneSeq(Glucose::Solver& solver, const std::vector<Glucose::Lit>& lits) {
    if (lits.size() <= 1) return;
    std::vector<Glucose::Lit> s(lits.size());
    for (size_t i = 0; i < lits.size(); ++i)
        s[i] = Glucose::mkLit(solver.newVar());

    // L[0] -> S[0]
    solver.addClause(~lits[0], s[0]);

    for (size_t i = 1; i < lits.size(); ++i) {
        // L[i] -> S[i]
        solver.addClause(~lits[i], s[i]);
        // S[i-1] -> S[i]
        solver.addClause(~s[i-1], s[i]);
        // (L[i] & S[i-1]) -> false
        solver.addClause(~lits[i], ~s[i-1]);
    }
}

// "Exactly one" = at least one + at most one
void addExactlyOneSeq(Glucose::Solver& solver, const std::vector<Glucose::Lit>& lits) {
    if (lits.empty()) return;
    Glucose::vec<Glucose::Lit> clause;
    for (auto v : lits) clause.push(v);
    solver.addClause(clause); // at least one
    addAtMostOneSeq(solver, lits); // at most one
}

bool HasMatchSat(const std::vector<Shape>& shapes,
              const std::vector<int>& counts,
              int W, int H,
              bool allowReflections = false)
{
    int64_t totalToPlace = 0;
    for (auto count : counts)
    {
        totalToPlace += count;
    }
    
    int64_t numFitX = W / 3;
    int64_t numFitY = H / 3;
    
    if (numFitX * numFitY > totalToPlace)
        return true;
    
    int64_t totalToFill = 0;
    for (const Shape& shape : shapes)
    {
        totalToPlace += shape.cells.size();
    }
    
    if (totalToFill > W * H)
        return false;
    
    
    // Generate orientations
    std::vector<std::vector<OrientedShape>> orientations(shapes.size());
    for (int t = 0; t < (int)shapes.size(); ++t)
        orientations[t] = generate_orientations(shapes[t], allowReflections);

    const int boardCols = W * H;
    constexpr int MAX_CELL = 100 * 100; // adjust if needed

    struct PlacementRow {
        std::bitset<MAX_CELL> coveredCells;
        int type, copyIndex, x, y;
        std::string orientationTag;
    };
    std::vector<PlacementRow> rows;
    rows.reserve(1000000);

    // Generate all placements
    for (int t = 0; t < (int)counts.size(); ++t) {
        if (counts[t] == 0) continue;
        for (const auto& o : orientations[t]) {
            for (int y = 0; y + o.h <= H; ++y) {
                for (int x = 0; x + o.w <= W; ++x) {
                    std::bitset<MAX_CELL> covered;
                    bool ok = true;
                    for (auto [cx, cy] : o.cells) {
                        int gx = x + cx, gy = y + cy;
                        if (gx < 0 || gy < 0 || gx >= W || gy >= H) { ok = false; break; }
                        covered.set(cellIndex(gx, gy, W));
                    }
                    if (!ok) continue;

                    for (int k = 0; k < counts[t]; ++k) {
                        PlacementRow pr;
                        pr.type = t;
                        pr.copyIndex = k;
                        pr.x = x; pr.y = y;
                        pr.orientationTag = o.tag;
                        pr.coveredCells = covered;
                        rows.push_back(std::move(pr));
                    }
                }
            }
        }
    }

    // Build SAT solver
    Glucose::Solver solver;
    std::vector<Glucose::Lit> placementVars(rows.size());
    for (size_t i = 0; i < rows.size(); ++i)
        placementVars[i] = Glucose::mkLit(solver.newVar());

    // Piece usage constraints
    std::map<std::pair<int,int>, std::vector<Glucose::Lit>> pieceMap;
    for (size_t i = 0; i < rows.size(); ++i)
        pieceMap[{rows[i].type, rows[i].copyIndex}].push_back(placementVars[i]);

    for (auto& [key, vars] : pieceMap)
        addExactlyOneSeq(solver, vars);

    // Board cell constraints
    std::vector<std::vector<Glucose::Lit>> cellMap(boardCols);
    for (size_t i = 0; i < rows.size(); ++i) {
        for (int idx = 0; idx < boardCols; ++idx)
            if (rows[i].coveredCells.test(idx))
                cellMap[idx].push_back(placementVars[i]);
    }
    for (int c = 0; c < boardCols; ++c)
        addAtMostOneSeq(solver, cellMap[c]);

    std::cout << "Vars: " << solver.nVars() << "\n";
    std::cout << "Clauses: " << solver.nClauses() << "\n" << std::flush;
    
    // Solve
    bool sat = solver.solve();
    if (!sat) {
        std::cout << "No solution exists.\n";
        return false;
    }

    return true;
}

void Day12(const std::string& InFileName, const int64_t InPart1Expected, const int64_t InPart2Expected)
{
    std::printf("Result for %s\n", InFileName.c_str());
    std::ifstream inputFile;
    inputFile.open(InFileName);
    std::string inputLine;
    vector<Shape> shapes;
    vector<ConstraintToEvaluate> evaluateBoxes;
    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        if (inputLine.empty())
            continue;
        else if (inputLine[inputLine.length() - 1] == ':')
        {
            std::string l1, l2, l3;
            std::getline(inputFile, l1);
            std::getline(inputFile, l2);
            std::getline(inputFile, l3);
            shapes.emplace_back(make_shape_from_ascii({
                l1, l2, l3
            }, inputLine.substr(0, inputLine.length() - 2)));
        }
        else if (!inputLine.empty())
        {
            ConstraintToEvaluate constraint;
            stringstream ss(inputLine);
            char delim;
            ss >> constraint.W >>delim >> constraint.H >> delim;
            int count;
            while (ss.peek() != EOF)
            {
                ss >> count;
                constraint.CountTargets.emplace_back(count);
            }
            evaluateBoxes.emplace_back(constraint);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    int64_t part1Count = 0;
    for (auto constraint : evaluateBoxes)
    {
        int64_t shapeCount = 0;
        for (int countTarget : constraint.CountTargets)
        {
            shapeCount += countTarget;
        }
        if (shapeCount * 8 < constraint.W * constraint.H)
            part1Count++;
    }
    
    int64_t part2Count = 0;

    auto end = std::chrono::high_resolution_clock::now();
    std::printf("Result part 1: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part1Count == InPart1Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::printf("Result part 2: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part2Count == InPart2Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    Day12("TestInput.txt", 2, 2); // Part 1 breaks with new Test Input
    Day12("Input.txt", 772,423227545768872);
    return 0;
}
