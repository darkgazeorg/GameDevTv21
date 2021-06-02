#include "MapGen.h"
#include "Types.h"

#include "Gorgon/Utils/Assert.h"
#include <Gorgon/Types.h>

#include <algorithm>
#include <cstdlib>
#include <ostream>

namespace {

    using UShapeMatrix = DirArray<DirArray<DirArray<Direction>>>;

    Direction getopposingdir(Direction dir) {
        static const DirArray<std::function<Direction()>> fns = {
            [] {return Direction::West;},
            [] {return Direction::East;},
            [] {return Direction::South;},
            [] {return Direction::North;},
        };
        return fns[dir]();
    }

    Direction resolvedirection(Point current, Point neighbor) {
        Direction dir = Direction::End;
        if(current.X != neighbor.X) {
            ASSERT(current.Y == neighbor.Y, "expected change only in X");
            dir = Gorgon::Sign(neighbor.X - current.X) > 0 ? Direction::East : Direction::West;
        }
        else {
            ASSERT(current.X == neighbor.X, "expected change only in Y");
            dir = Gorgon::Sign(neighbor.Y - current.Y) > 0 ? Direction::South : Direction::North;
        }
        return dir;
    }

    UShapeMatrix initushapematrix() {
        UShapeMatrix ushapes;
        for(int i = 0; i < DIR_SIZE; i++) {
            for(int j = 0; j < DIR_SIZE; j++) {
                for(int k = 0; k < DIR_SIZE; k++) {
                    ushapes[i][j][k] = Direction::End;
                }
            }
        }
        // TODO: the first and the third steps are not necessary, only store the second step
        // and change the query on the call site accordingly
        ushapes[Direction::East][Direction::North][Direction::West] = Direction::North;
        ushapes[Direction::East][Direction::South][Direction::West] = Direction::South;
        ushapes[Direction::West][Direction::North][Direction::East] = Direction::North;
        ushapes[Direction::West][Direction::South][Direction::East] = Direction::South;
        ushapes[Direction::North][Direction::East][Direction::South] = Direction::East;
        ushapes[Direction::North][Direction::West][Direction::South] = Direction::West;
        ushapes[Direction::South][Direction::East][Direction::North] = Direction::East;
        ushapes[Direction::South][Direction::West][Direction::North] = Direction::West;
        return ushapes;
    }

    std::pair<Direction, Direction> findtwoclosestedges(Point point, Size size) {
        std::pair<Direction, Direction> dirs;
        dirs.first = point.X >= size.Width / 2 ? Direction::East : Direction::West;
        dirs.second = point.Y >= size.Height / 2 ? Direction::South : Direction::North;
        return dirs;
    }

    std::vector<Point> normalizepath(std::vector<Point> path) {
        ASSERT(path.size() > 0, "empty path");
        const auto sortx = [] (const Point& lhs, const Point& rhs) { return lhs.X < rhs.X; };
        const auto sorty = [] (const Point& lhs, const Point& rhs) { return lhs.Y < rhs.Y; };
        int minx = (*std::min_element(path.begin(), path.end(), sortx)).X;
        int miny = (*std::min_element(path.begin(), path.end(), sorty)).Y;
        for(auto& point : path) {
            point.X += -minx;
            point.Y += -miny;
        }
        return path;
    }

    std::ostream& operator<<(std::ostream& out, Direction dir) {
        static const DirArray<std::function<const char*()>> printers = {
            [] () {return "East";},
            [] () {return "West";},
            [] () {return "North";},
            [] () {return "South";},
        };
        out << printers[dir]();
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Walls& walls) {
        out << "W: " << walls[Direction::West]
            << ", N: " << walls[Direction::North]
            << ", E: " << walls[Direction::East]
            << ", S: " << walls[Direction::South];
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Cell& cell) {
        out << "coord: " << cell.coord << ", " << cell.walls;
        return out;
    }
}

std::ostream& operator<<(std::ostream& out, const RecursiveBacktracker::CellData& data) {
    out << data.cell << ", visited? " << data.visited;
    return out;
}

std::vector<Point> StretchUTurns(std::vector<Point> orgpath) {
    static const UShapeMatrix ushapes = initushapematrix();
    static const DirArray<Point> shiftamounts = {
        Point(1, 0),
        Point(-1, 0),
        Point(0, -1),
        Point(0, 1),
    };
    std::vector<Point> path;
    path.reserve(orgpath.size() + orgpath.size() / 3);
    orgpath.reserve(path.capacity());
    constexpr int stepsize = 3;
    for(auto current = orgpath.begin(); current != orgpath.end(); ++current) {
        path.push_back(*current);
        if(std::distance(current, orgpath.end()) <= stepsize) {
            continue;
        }
        auto next = current + 1;
        auto afternext = current + 2;
        Direction stretchdir = ushapes[resolvedirection(*current, *next)]
                                      [resolvedirection(*next, *afternext)]
                                      [resolvedirection(*afternext, *(current + 3))];
        if(stretchdir != Direction::End) {
            Point shift = shiftamounts[stretchdir];
            static const DirArray<std::function<bool(Point, Point)>> shiftcheckers = {
                [] (Point current, Point point) {return point.X > current.X;},
                [] (Point current, Point point) {return point.X < current.X;},
                [] (Point current, Point point) {return point.Y < current.Y;},
                [] (Point current, Point point) {return point.Y > current.Y;},
            };
            using Iter = std::vector<Point>::iterator;
            const auto shifter = [stretchdir, current] (Iter start, Iter end, Point shift) {
                for(auto it = start; it != end; ++it) {
                    if(shiftcheckers[stretchdir](*current, *it)) {
                        *it += shift;
                    }
                }
            };
            // TODO: shifting will leave gaps, fill them!
            shifter(path.begin(), path.end(), shift); // shift backwards
            shifter(afternext, orgpath.end(), shift); // shift forward
        }
    }
    return normalizepath(path);
}

std::vector<Point> ConnectEnteranceToEdge(std::vector<Point> path, Size pathsize, Size mapsize) {
    ASSERT(path.size() > 0, "invalid path");
    auto enterance = path.begin();
    std::pair<Direction, Direction> dirs = findtwoclosestedges(*enterance, mapsize);
    const auto coldetector = [&path] (Point current, Direction dir) {
        static const DirArray<std::function<bool(Point, Point)>> detectors = {
            [] (Point current, Point other) {return other.X > current.X && other.Y == current.Y;},
            [] (Point current, Point other) {return other.X < current.X && other.Y == current.Y;},
            [] (Point current, Point other) {return other.X == current.X && other.Y < current.Y;},
            [] (Point current, Point other) {return other.X == current.X && other.Y > current.Y;},
        };
        return std::find_if(path.begin(), path.end(), [current, dir] (const Point& other) {
            return detectors[dir](current, other);
        }) != path.end();
    };
    static const DirArray<std::function<int(Point, Size, Size)>> tilecounters = {
        [] (Point enterance, Size pathsize, Size mapsize)
            {return ((pathsize.Width - 1) - enterance.X) + ((mapsize.Width - pathsize.Width) / 2 + 1);},
        [] (Point enterance, Size pathsize, Size mapsize)
            {return (enterance.X) + ((mapsize.Width - pathsize.Width) / 2 + 1);},
        [] (Point enterance, Size pathsize, Size mapsize)
            {return (enterance.Y) + ((mapsize.Height - pathsize.Height) / 2 + 1);},
        [] (Point enterance, Size pathsize, Size mapsize)
            {return ((pathsize.Height - 1) - enterance.Y) + ((mapsize.Height - pathsize.Height) / 2 + 1);},
    };
    bool iscol1stedge = coldetector(*enterance, dirs.first);
    bool iscol2ndedge = coldetector(*enterance, dirs.second);
    ASSERT(!(iscol1stedge && iscol2ndedge), "collision in both direction");
    Direction dir = !iscol1stedge ? dirs.first : dirs.second;
    auto current = enterance;
    const int tilecount = tilecounters[dir](*enterance, pathsize, mapsize);
    // !!! this *may* insert more tiles than necessary
    for(int i = 0; i < tilecount; i++) {
        Point currpoint = RecursiveBacktracker::getneighbortowards(*current, dir);
        current = path.insert(current, currpoint);
    }
    return path;
}

Point RecursiveBacktracker::getneighbortowards(Point coord, Direction dir) {
    const DirArray<std::function<Point()>> fns = {
        [coord] {return Point(coord.X + 1, coord.Y);},
        [coord] {return Point(coord.X - 1, coord.Y);},
        [coord] {return Point(coord.X, coord.Y - 1);},
        [coord] {return Point(coord.X, coord.Y + 1);},
    };
    return fns[dir]();
}

void RecursiveBacktracker::clear(Size size) {
    ASSERT(size.Area() > 0, "invalid size");
    this->size = size;
    cells.reserve(size.Area());
    cells.clear();
    for(int y = 0; y < size.Height; y++) {
        for(int x = 0; x < size.Width; x++) {
            cells.push_back({{{x, y},
                              {true, true, true, true}},
                             false});
            // TODO: remove the following assertion
            ASSERT(cells.size() - 1 == in1d({x, y}), "cell at incorrect index");
        }
    }
}

bool RecursiveBacktracker::checkbounds(Point coord, Direction dir) const {
    const DirArray<std::function<bool()>> fns = {
        [coord, this] {return coord.X < size.Width - 1;},
        [coord] {return coord.X > 0;},
        [coord] {return coord.Y > 0;},
        [coord, this] {return coord.Y < size.Height - 1;},
    };
    return fns[dir]();
}

void RecursiveBacktracker::removewall(Cell& current, Cell& neighbor)
{
    Direction dir = resolvedirection(current.coord, neighbor.coord);
    Direction opposingdir = getopposingdir(dir);
    current.walls[dir] = false;
    neighbor.walls[opposingdir] = false;
}

std::pair<Point, Point> RecursiveBacktracker::genstartandend() {
    Direction startedge = (Direction)(std::rand() % 4);
    Direction endedge = getopposingdir(startedge);

    const DirArray<std::function<Point()>> coordgenerators = {
        [this] {return Point(size.Width - 1, std::rand() % size.Height);},
        [this] {return Point(0, std::rand() % size.Height);},
        [this] {return Point(std::rand() % size.Width, 0);},
        [this] {return Point(std::rand() % size.Width, size.Height - 1);},
    };

    Point enterance = coordgenerators[startedge]();
    ASSERT(enterance.X >= 0 && enterance.Y >= 0, "invalid coordinate");
    Point exit = coordgenerators[endedge]();
    ASSERT(exit.X >= 0 && exit.Y >= 0, "invalid coordinate");
    return {enterance, exit};
}

std::vector<Point> RecursiveBacktracker::findpassableneighbors(const Cell& current) const {
    auto fn = [&current, this] (Point neighbor) {
        return !isvisited(neighbor) && !current.walls[resolvedirection(current.coord, neighbor)];
    };
    return findneighborsimpl(current.coord, fn);
}

Maze RecursiveBacktracker::Generate(Size size) {
    clear(size);
    std::vector<CellData*> stack;
    CellData& first = cells[std::rand() % cells.size()];
    first.visited = true;
    stack.push_back(&first);
    while(!stack.empty()) {
        CellData& curr = *stack.back();
        stack.pop_back();
        std::vector<Point> coords = findunvisitedneighbors(curr.cell.coord);
        std::size_t numofneighbors = coords.size();
        if(numofneighbors > 0) {
            stack.push_back(&curr);
            Point coord = coords[std::rand() % numofneighbors];
            CellData& neighbor = cells[in1d(coord)];
            ASSERT(
                coord.X >= 0 && coord.X < size.Width &&
                coord.Y >= 0 && coord.Y < size.Height,
                "neighbor out of bounds");
            removewall(curr.cell, neighbor.cell);
            neighbor.visited = true;
            stack.push_back(&neighbor);
        }
    }
    std::vector<Cell> maze;
    for(const auto& cell: cells) {
        ASSERT(cell.cell.coord.X >= 0 &&cell.cell.coord.Y >= 0, "invalid coordinate");
        maze.push_back(cell.cell);
    }
    return {genstartandend(), maze};
}

std::vector<Point> RecursiveBacktracker::Solve(const Maze& maze, Size size) {
    clear(size);
    std::vector<Point> solution;
    solution.push_back(maze.first.first);
    while(true) {
        Point current = solution.back();
        if(current == maze.first.second) {
            break;
        }
        std::vector<Point> neigbors = findpassableneighbors(maze.second[in1d(current)]);
        if(neigbors.size() > 0) {
            solution.push_back(neigbors[0]);
        }
        else {
            solution.pop_back();
        }
        cells[in1d(current)].visited = true;
    }
    return solution;
}
