#pragma once

#include "Types.h"

#include <Gorgon/Utils/Assert.h>

#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using Walls = std::unordered_map<Direction, bool>;
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
    RecursiveBacktracker(int width, int height);

    std::vector<Cell> Generate();

private:
    Size size;
    std::vector<CellData> cells;

    int in1d(Point coord) const {
        return coord.Y * size.Width + coord.X;
    }

    bool isvisited(Point coord) const {
        return cells[in1d(coord)].visited;
    }

    void removewall(Cell& cell, Cell& neighbor);

    bool checkbounds(Point coord, Direction dir) const;

    std::vector<Point> findunvisitedneighbors(Point current) const;
};
