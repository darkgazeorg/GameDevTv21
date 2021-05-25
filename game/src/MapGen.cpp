#include "MapGen.h"
#include "Types.h"

#include "Gorgon/Utils/Assert.h"
#include <Gorgon/Types.h>

#include <chrono>
#include <cstdlib>
#include <exception>
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

RecursiveBacktracker::RecursiveBacktracker(int width, int height)
    : size(width, height)
{
    ASSERT(size.Area() > 0, "invalid size");
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
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

std::vector<Point> RecursiveBacktracker::findunvisitedneighbors(Point current) const {
    std::vector<Point> unvisited;
    std::vector<std::pair<bool, Point>> neighbors = {
        {checkbounds(current, Direction::West), getneighbortowards(current, Direction::West)},
        {checkbounds(current, Direction::North), getneighbortowards(current, Direction::North)},
        {checkbounds(current, Direction::East), getneighbortowards(current, Direction::East)},
        {checkbounds(current, Direction::South), getneighbortowards(current, Direction::South)}
    };
    for(const auto& neighbor : neighbors) {
        bool withinbounds = neighbor.first;
        Point coord = neighbor.second;
        if(withinbounds && !isvisited(coord)) {
            unvisited.push_back(neighbor.second);
        }
    }
    return unvisited;
}

void RecursiveBacktracker::removewall(Cell& current, Cell& neighbor)
{
    Direction dir = Direction::North;
    if(current.coord.X != neighbor.coord.X) {
        ASSERT(current.coord.Y == neighbor.coord.Y, "expected change only in X");
        dir = Gorgon::Sign(neighbor.coord.X - current.coord.X) > 0 ? Direction::East : Direction::West;
    }
    else {
        ASSERT(current.coord.X == neighbor.coord.X, "expected change only in Y");
        dir = Gorgon::Sign(neighbor.coord.Y - current.coord.Y) > 0 ? Direction::South : Direction::North;
    }
    ASSERT_WALL_EXISTS(current.walls, dir);
    Direction opposingdir = getopposingdir(dir);
    ASSERT_WALL_EXISTS(current.walls, opposingdir);
    current.walls[dir] = false;
    neighbor.walls[opposingdir] = false;
}

std::vector<Cell> RecursiveBacktracker::Generate() {
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
    return maze;
}
