#include "MapGen.h"
#include "Types.h"

#include "Gorgon/Utils/Assert.h"
#include <Gorgon/Types.h>

#include <cstdlib>
#include <functional>
#include <ostream>

#define ASSERT_WALL_EXISTS(walls, dir) ASSERT(walls.count(dir) > 0, "wall does not exist")

namespace {
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

Point RecursiveBacktracker::getneighbortowards(Point coord, Direction dir) {
    const std::unordered_map<Direction, std::function<Point()>> fns = {
        {Direction::East, [coord] {return Point(coord.X + 1, coord.Y);}},
        {Direction::West, [coord] {return Point(coord.X - 1, coord.Y);}},
        {Direction::South, [coord] {return Point(coord.X, coord.Y + 1);}},
        {Direction::North, [coord] {return Point(coord.X, coord.Y - 1);}}
    };
    return RecursiveBacktracker::executeperdir(dir, fns);
}

Direction RecursiveBacktracker::getopposingdir(Direction dir) {
    static const std::unordered_map<Direction, std::function<Direction()>> fns = {
        {Direction::East, [] {return Direction::West;}},
        {Direction::West, [] {return Direction::East;}},
        {Direction::South, [] {return Direction::North;}},
        {Direction::North, [] {return Direction::South;}}
    };
    return executeperdir(dir, fns);
}

Direction RecursiveBacktracker::resolvedirection(Point current, Point neighbor) {
    Direction dir = Direction::North;
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
    return RecursiveBacktracker::executeperdir(dir, fns);
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

    return {executeperdir(startedge, coordgenerators), executeperdir(endedge, coordgenerators)};
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
