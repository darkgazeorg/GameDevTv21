#include "MapGen.h"
#include "Types.h"

#include "Gorgon/Utils/Assert.h"
#include <Gorgon/Types.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ostream>

#define ASSERT_WALL_EXISTS(walls, dir) ASSERT(walls.count(dir) > 0, "wall does not exist")

namespace {
    template<typename T, int N>
    using Arr = std::array<T, N>;
    constexpr int dirsize = (int)Direction::End;
    using UShapeMatrix = Arr<Arr<Arr<Direction, dirsize>, dirsize>, dirsize>;

    template<typename Func, typename... Args>
    static auto executeperdir(
        Direction dir,
        const std::unordered_map<Direction, Func>& funcs,
        Args... args) {
        switch(dir) {
        case Direction::East:
            return funcs.at(Direction::East)(args...);
            break;
        case Direction::West:
            return funcs.at(Direction::West)(args...);
            break;
        case Direction::South:
            return funcs.at(Direction::South)(args...);
            break;
        case Direction::North:
            return funcs.at(Direction::North)(args...);
            break;
        default:
            Gorgon::Utils::ASSERT_FALSE("Unknown direction");
        }
        typename std::result_of<Func(Args...)>::type ret{};
        return ret;
    }

    Direction getopposingdir(Direction dir) {
        static const std::unordered_map<Direction, std::function<Direction()>> fns = {
            {Direction::East, [] {return Direction::West;}},
            {Direction::West, [] {return Direction::East;}},
            {Direction::South, [] {return Direction::North;}},
            {Direction::North, [] {return Direction::South;}}
        };
        return executeperdir(dir, fns);
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
        constexpr int end = (int)Direction::End;
        for(int i = 0; i < end; i++) {
            for(int j = 0; j < end; j++) {
                for(int k = 0; k < end; k++) {
                    ushapes[i][j][k] = Direction::End;
                }
            }
        }
        constexpr int east = (int)Direction::East;
        constexpr int west = (int)Direction::West;
        constexpr int north = (int)Direction::North;
        constexpr int south = (int)Direction::South;
        ushapes[east][north][west] = Direction::North;
        ushapes[east][south][west] = Direction::South;
        ushapes[west][north][east] = Direction::North;
        ushapes[west][south][east] = Direction::South;
        ushapes[north][east][south] = Direction::East;
        ushapes[north][west][south] = Direction::West;
        ushapes[south][east][north] = Direction::East;
        ushapes[south][west][north] = Direction::West;
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
        static const std::unordered_map<Direction, std::function<std::string()>> printers = {
            {Direction::East, [] () {return "East";}},
            {Direction::West, [] () {return "West";}},
            {Direction::North, [] () {return "North";}},
            {Direction::South, [] () {return "South";}}
        };
        out << executeperdir(dir, printers);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Walls& walls) {
        ASSERT_WALL_EXISTS(walls, Direction::West);
        ASSERT_WALL_EXISTS(walls, Direction::North);
        ASSERT_WALL_EXISTS(walls, Direction::East);
        ASSERT_WALL_EXISTS(walls, Direction::South);
        out << "W: " << walls.at(Direction::West)
            << ", N: " << walls.at(Direction::North)
            << ", E: " << walls.at(Direction::East)
            << ", S: " << walls.at(Direction::South);
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
    static const std::array<Point, (int)Direction::End> shiftamounts = {
        Point(0, -1),
        Point(0, 1),
        Point(1, 0),
        Point(-1, 0),
    };
    std::vector<Point> path;
    path.reserve(orgpath.size() + orgpath.size() / 3);
    orgpath.reserve(path.capacity());
    constexpr int stepsize = 3;
    for(auto current = orgpath.begin(); current != orgpath.end(); ++current) {
        auto next = current + 1;
        auto afternext = current + 2;
        path.push_back(*current);
        if(std::distance(current, orgpath.end()) <= stepsize) {
            continue;
        }
        Direction stretchdir = ushapes[(int)resolvedirection(*current, *next)]
                                      [(int)resolvedirection(*next, *afternext)]
                                      [(int)resolvedirection(*afternext, *(current + 3))];
        if(stretchdir != Direction::End) {
            Point shift = shiftamounts[(int)stretchdir];
            static const std::unordered_map<Direction, std::function<bool(Point, Point)>> shiftcheckers = {
                {Direction::East, [] (Point current, Point point) {return point.X > current.X;}},
                {Direction::West, [] (Point current, Point point) {return point.X < current.X;}},
                {Direction::North, [] (Point current, Point point) {return point.Y < current.Y;}},
                {Direction::South, [] (Point current, Point point) {return point.Y > current.Y;}}
            };
            using Iter = std::vector<Point>::iterator;
            const auto shifter = [stretchdir, current] (Iter start, Iter end, Point shift) {
                for(auto it = start; it != end; ++it) {
                    if(executeperdir(stretchdir, shiftcheckers, *current, *it)) {
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
        static const std::unordered_map<Direction, std::function<bool(Point, Point)>> detectors = {
            {Direction::East, [] (Point current, Point other) {return other.X > current.X && other.Y == current.Y;}},
            {Direction::West, [] (Point current, Point other) {return other.X < current.X && other.Y == current.Y;}},
            {Direction::North, [] (Point current, Point other) {return other.X == current.X && other.Y < current.Y;}},
            {Direction::South, [] (Point current, Point other) {return other.X == current.X && other.Y > current.Y;}},
        };
        return std::find_if(path.begin(), path.end(), [current, dir] (const Point& other) {
            return detectors.at(dir)(current, other);
        }) != path.end();
    };
    static const std::unordered_map<Direction, std::function<int(Point, Size, Size)>> tilecounters = {
        {Direction::East, [] (Point enterance, Size pathsize, Size mapsize)
            {return ((pathsize.Width - 1) - enterance.X) + ((mapsize.Width - pathsize.Width) / 2 + 1);}},
        {Direction::West, [] (Point enterance, Size pathsize, Size mapsize)
            {return (enterance.X) + ((mapsize.Width - pathsize.Width) / 2 + 1);}},
        {Direction::North, [] (Point enterance, Size pathsize, Size mapsize)
            {return (enterance.Y) + ((mapsize.Height - pathsize.Height) / 2 + 1);}},
        {Direction::South, [] (Point enterance, Size pathsize, Size mapsize)
            {return ((pathsize.Height - 1) - enterance.Y) + ((mapsize.Height - pathsize.Height) / 2 + 1);}},
    };
    bool iscol1stedge = coldetector(*enterance, dirs.first);
    bool iscol2ndedge = coldetector(*enterance, dirs.second);
    ASSERT(!(iscol1stedge && iscol2ndedge), "collision in both direction");
    Direction dir = !iscol1stedge ? dirs.first : dirs.second;
    auto current = enterance;
    const int tilecount = tilecounters.at(dir)(*enterance, pathsize, mapsize);
    // !!! this *may* insert more tiles than necessary
    for(int i = 0; i < tilecount; i++) {
        Point currpoint = RecursiveBacktracker::getneighbortowards(*current, dir);
        current = path.insert(current, currpoint);
    }
    return path;
}

Point RecursiveBacktracker::getneighbortowards(Point coord, Direction dir) {
    const std::unordered_map<Direction, std::function<Point()>> fns = {
        {Direction::East, [coord] {return Point(coord.X + 1, coord.Y);}},
        {Direction::West, [coord] {return Point(coord.X - 1, coord.Y);}},
        {Direction::South, [coord] {return Point(coord.X, coord.Y + 1);}},
        {Direction::North, [coord] {return Point(coord.X, coord.Y - 1);}}
    };
    return executeperdir(dir, fns);
}

void RecursiveBacktracker::clear(Size size) {
    ASSERT(size.Area() > 0, "invalid size");
    this->size = size;
    cells.reserve(size.Area());
    cells.clear();
    for(int y = 0; y < size.Height; y++) {
        for(int x = 0; x < size.Width; x++) {
            cells.push_back({{{x, y},
                              {{Direction::East, true},
                               {Direction::West, true},
                               {Direction::North, true},
                               {Direction::South, true}}},
                             false});
            // TODO: remove the following assertion
            ASSERT(cells.size() - 1 == in1d({x, y}), "cell at incorrect index");
        }
    }
}

bool RecursiveBacktracker::checkbounds(Point coord, Direction dir) const {
    const std::unordered_map<Direction, std::function<bool()>> fns = {
        {Direction::East, [coord, this] {return coord.X < size.Width - 1;}},
        {Direction::West, [coord] {return coord.X > 0;}},
        {Direction::South, [coord, this] {return coord.Y < size.Height - 1;}},
        {Direction::North, [coord] {return coord.Y > 0;}}
    };
    return executeperdir(dir, fns);
}

void RecursiveBacktracker::removewall(Cell& current, Cell& neighbor)
{
    Direction dir = resolvedirection(current.coord, neighbor.coord);
    ASSERT_WALL_EXISTS(current.walls, dir);
    Direction opposingdir = getopposingdir(dir);
    ASSERT_WALL_EXISTS(current.walls, opposingdir);
    current.walls[dir] = false;
    neighbor.walls[opposingdir] = false;
}

std::pair<Point, Point> RecursiveBacktracker::genstartandend() {
    Direction startedge = (Direction)(std::rand() % 4);
    Direction endedge = getopposingdir(startedge);

    std::unordered_map<Direction, std::function<Point()>> coordgenerators = {
        {Direction::East, [this] {return Point(size.Width - 1, std::rand() % size.Height);}},
        {Direction::West, [this] {return Point(0, std::rand() % size.Height);}},
        {Direction::South, [this] {return Point(std::rand() % size.Width, size.Height - 1);}},
        {Direction::North, [this] {return Point(std::rand() % size.Width, 0);}}
    };

    Point enterance = executeperdir(startedge, coordgenerators);
    ASSERT(enterance.X >= 0 && enterance.Y >= 0, "invalid coordinate");
    Point exit = executeperdir(endedge, coordgenerators);
    ASSERT(exit.X >= 0 && exit.Y >= 0, "invalid coordinate");
    return {enterance, exit};
}

std::vector<Point> RecursiveBacktracker::findpassableneighbors(const Cell& current) const {
    auto fn = [&current, this] (Point neighbor) {
        return !isvisited(neighbor) && !current.walls.at(resolvedirection(current.coord, neighbor));
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
