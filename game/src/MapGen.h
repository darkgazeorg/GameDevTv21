#pragma once

#include "Types.h"

#include <Gorgon/Utils/Assert.h>

#include <vector>
#include <utility>
#include <unordered_map>
#include <type_traits>

class RecursiveBacktracker {
    enum class Direction {
        North,
        South,
        East,
        West,
    };

    static Point getneighbortowards(Point coordinate, Direction dir);

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
    using CarveInstructions = std::vector<std::pair<Point, Direction>>;

    RecursiveBacktracker(int width, int height, int walloffset = 1)
        : walloffset(walloffset)
        , bounds(Point(walloffset, walloffset), Point(width - (walloffset + 1), height - (walloffset + 1)))
        , size(width, height)
        , visitedcells(Size(width - (walloffset * 2), height - (walloffset * 2)).Area(), 0)
    {
        ASSERT(size.Area() > 0, "invalid size");
    }

    CarveInstructions Generate();

private:
    int walloffset;
    Bounds bounds; // maze bounds, excluding outer walls
    Size size; // full maze size, including outer walls
    std::vector<int> visitedcells;
    CarveInstructions carveinstr;

    int in1d(Point coord) const {
        return (coord.Y - walloffset) * bounds.Right + (coord.X - walloffset);
    }

    void markvisited(Point coord) {
        visitedcells[in1d(coord)] = 1;
    }

    bool checkbounds(Point coord, Direction dir) const;
    std::vector<Point> findunvisitedneighbors(Point current) const;
    void removewall(Point current, Point neighbor);
    void addenteranceandexit();
};
