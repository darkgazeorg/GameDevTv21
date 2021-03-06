#pragma once

#include "Types.h"

#include <Gorgon/Utils/Assert.h>

#include <array>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <vector>

constexpr int DIR_SIZE = (int)Direction::End;

template<typename T>
struct DirArray {
    const T& operator[](Direction dir) const {
        ASSERT(dir != Direction::End, "not a valid index");
        return operator[]((int)dir);
    }

    T& operator[](Direction dir) {
        ASSERT(dir != Direction::End, "not a valid index");
        return operator[]((int)dir);
    }

    const T& operator[](int dir) const {
        return arr[dir];
    }

    T& operator[](int dir) {
        return arr[dir];
    }

    std::array<T, DIR_SIZE> arr;
};

using Walls = DirArray<bool>;
struct Cell;
using Maze = std::pair<std::pair<Point, Point>, std::vector<Cell>>;

struct Cell {
    Point coord;
    Walls walls;
};

std::vector<Point> StretchUTurns(std::vector<Point> orgpath);
std::vector<Point> ConnectEnteranceToEdge(std::vector<Point> orgpath, Size pathsize, Size mapsize);

class RecursiveBacktracker {
    struct CellData {
        Cell cell;
        bool visited;
    };

    friend std::ostream& operator<<(std::ostream& out, const CellData& data);

public:
    Maze Generate(Size size);
    std::vector<Point> Solve(const Maze& maze, Size size);

    static Point getneighbortowards(Point coordinate, Direction dir);
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

class PathChecker {
    using Constraint = std::function<bool(const std::vector<Point>&)>;

public:
    PathChecker(const std::initializer_list<Constraint>& constraints) {
        for(const auto& constraint : constraints) {
            this->constraints.push_back(constraint);
        }
    }

    bool Check(const std::vector<Point>& path) const {
        for(const auto& constraint: constraints) {
            if(!constraint(path)) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<Constraint> constraints;
};
