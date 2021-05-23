#include "MapGen.h"

#include <functional>

Point RecursiveBacktracker::getneighbortowards(Point coord, Direction dir) {
    const std::unordered_map<Direction, std::function<Point()>> fns = {
        {Direction::East, [coord] {return Point(coord.X + 1, coord.Y);}},
        {Direction::West, [coord] {return Point(coord.X - 1, coord.Y);}},
        {Direction::South, [coord] {return Point(coord.X, coord.Y + 1);}},
        {Direction::North, [coord] {return Point(coord.X, coord.Y - 1);}}
    };
    return RecursiveBacktracker::executeperdir(dir, fns);
}

bool RecursiveBacktracker::checkbounds(Point coord, Direction dir) const {
    const std::unordered_map<Direction, std::function<bool()>> fns = {
        {Direction::East, [coord, right = bounds.Right] {return coord.X < right;}},
        {Direction::West, [coord, left = bounds.Left] {return coord.X > left;}},
        {Direction::South, [coord, bottom = bounds.Bottom] {return coord.Y < bottom;}},
        {Direction::North, [coord, top = bounds.Top] {return coord.Y > top;}}
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
        if(withinbounds && !visitedcells.at(in1d(coord))) {
            unvisited.push_back(neighbor.second);
        }
    }
    return unvisited;
}

void RecursiveBacktracker::removewall(Point current, Point neighbor)
{
    Direction dir = Direction::North;
    if(current.X != neighbor.X) {
        ASSERT(current.Y == neighbor.Y, "expected change only in X");
        dir = Gorgon::Sign(neighbor.X - current.X) > 0 ? Direction::East : Direction::West;
    }
    else {
        ASSERT(current.X == neighbor.X, "expected change only in Y");
        dir = Gorgon::Sign(neighbor.Y - current.Y) > 0 ? Direction::South : Direction::North;
    }
    carveinstr.push_back({current, dir});
}

void RecursiveBacktracker::addenteranceandexit() {
    static const std::unordered_map<Direction, Direction> endedges = {
        {Direction::East, Direction::West},
        {Direction::West, Direction::East},
        {Direction::North, Direction::South},
        {Direction::South, Direction::North}
    };
    Direction startedge = (Direction)(std::rand() % 4);
    Direction endedge = endedges.at(startedge);

    std::unordered_map<Direction, std::function<Point()>> coordgenerators = {
        {Direction::East, [end = size.Width - 1, height = size.Height] {return Point(end, std::rand() % height);}},
        {Direction::West, [height = size.Height] {return Point(0, std::rand() % height);}},
        {Direction::South, [end = size.Height - 1, width = size.Width] {return Point(std::rand() % width, end);}},
        {Direction::North, [width = size.Width] {return Point(std::rand() % width, 0);}}
    };
    Point start = executeperdir(startedge, coordgenerators);
    Point end = executeperdir(endedge, coordgenerators);

    std::unordered_map<Direction, std::function<void*(Point coord)>> carver = {
        {Direction::East, [&] (Point coord) {removewall(coord, getneighbortowards(coord, Direction::West)); return nullptr;}},
        {Direction::West, [&] (Point coord) {removewall(coord, getneighbortowards(coord, Direction::East)); return nullptr;}},
        {Direction::South, [&] (Point coord) {removewall(coord, getneighbortowards(coord, Direction::North)); return nullptr;}},
        {Direction::North, [&] (Point coord) {removewall(coord, getneighbortowards(coord, Direction::South)); return nullptr;}}
    };

    executeperdir(startedge, carver, start);
    executeperdir(endedge, carver, end);
}

RecursiveBacktracker::CarveInstructions RecursiveBacktracker::Generate() {
    std::vector<Point> cells;
    Point start((std::rand() % bounds.Right) + 1, (std::rand() % bounds.Bottom) + 1);
    cells.push_back(start);
    markvisited(start);
    while(!cells.empty()) {
        Point curr = cells.back();
        cells.pop_back();
        std::vector<Point> unvisitedneighbors = findunvisitedneighbors(curr);
        std::size_t numofneighbors = unvisitedneighbors.size();
        if(numofneighbors > 0) {
            cells.push_back(curr);
            Point neighbor = unvisitedneighbors[std::rand() % numofneighbors];
            ASSERT(
                neighbor.X >= bounds.Left && neighbor.X <= bounds.Right &&
                neighbor.Y >= bounds.Top  && neighbor.Y <= bounds.Bottom,
                "neighbor out of bounds");
            removewall(curr, neighbor);
            markvisited(neighbor);
            cells.push_back(neighbor);
        }
    }
    addenteranceandexit();
    return carveinstr;
}
