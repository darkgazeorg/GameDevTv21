#pragma once

#include "Types.h"

#include <Gorgon/Utils/Assert.h>

#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using Walls = std::unordered_map<Direction, bool>;
struct Cell;
using Maze = std::pair<std::pair<Point, Point>, std::vector<Cell>>;

struct Cell {
    Point coord;
    Walls walls;
};

class RecursiveBacktracker {
    struct CellData {
        Cell cell;
        bool visited;
    };

    friend std::ostream& operator<<(std::ostream& out, const CellData& data);

    static Point getneighbortowards(Point coordinate, Direction dir);
    static Direction getopposingdir(Direction dir);
    static Direction resolvedirection(Point current, Point target);

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

public:
    Maze Generate(Size size);
    std::vector<Point> Solve(const Maze& maze, Size size);

private:
    Size size;
    std::vector<CellData> cells;

    int in1d(Point coord) const {
        return coord.Y * size.Width + coord.X;
    }

    bool isvisited(Point coord) const {
        return cells[in1d(coord)].visited;
    }

    void clear(Size size);

    std::pair<Point, Point> genstartandend();

    void removewall(Cell& cell, Cell& neighbor);

    bool checkbounds(Point coord, Direction dir) const;

    std::vector<Point> findpassableneighbors(const Cell& current) const;

    std::vector<Point> findunvisitedneighbors(Point current) const {
        return findneighborsimpl(current, [this] (Point coord) { return !isvisited(coord);});
    }

    template<typename T>
    std::vector<Point> findneighborsimpl(Point current, T constraint) const {
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
            if(withinbounds && constraint(coord)) {
                unvisited.push_back(neighbor.second);
            }
        }
        return unvisited;
    }
};
