#include "MapGen.h"
#include <Gorgon/Utils/Assert.h>

namespace {
    enum class Direction {
        North,
        South,
        East,
        West,
    };

    int Get1DCoordinate(Point coordinate, int width) {
        return coordinate.Y * width + coordinate.X;
    }

    Point GetNeighborTowards(Point coordinate, Direction dir) {
        switch(dir) {
        case Direction::East:
            return {coordinate.X + 1, coordinate.Y};
            break;
        case Direction::West:
            return {coordinate.X - 1, coordinate.Y};
            break;
        case Direction::South:
            return {coordinate.X, coordinate.Y + 1};
            break;
        case Direction::North:
            return {coordinate.X, coordinate.Y - 1};
            break;
        default:
            Gorgon::Utils::ASSERT_FALSE("Unknown direction");
        }

        return {};
    }

    bool CheckDirection(Point coordinate, Direction dir, int width, int height) {
        switch(dir) {
        case Direction::East:
            return coordinate.X < width - 1;
            break;
        case Direction::West:
            return coordinate.X > 0;
            break;
        case Direction::South:
            return coordinate.Y < height - 1;
            break;
        case Direction::North:
            return coordinate.Y > 0;
            break;
        default:
            Gorgon::Utils::ASSERT_FALSE("Unknown direction");
        }

        return false;
    }

    std::vector<Point> FindUnvisitedNeighbors(const std::vector<int>& visited, Point current, int width, int height) {
        std::vector<Point> unvisited;
        std::vector<std::pair<bool, Point>> neighbors = {
            {CheckDirection(current, Direction::West, width, height), GetNeighborTowards(current, Direction::West)},
            {CheckDirection(current, Direction::North, width, height), GetNeighborTowards(current, Direction::North)},
            {CheckDirection(current, Direction::East, width, height), GetNeighborTowards(current, Direction::East)},
            {CheckDirection(current, Direction::South, width, height), GetNeighborTowards(current, Direction::South)}
        };
        for(const auto& neighbor : neighbors) {
            auto withinbounds = neighbor.first;
            auto coordinates = neighbor.second;
            if(withinbounds && !visited[Get1DCoordinate(coordinates, width)]) {
                unvisited.push_back(neighbor.second);
            }
        }
        return unvisited;
    }

    void RemoveWall(std::vector<std::pair<Point, Direction>>& map, Point current, Point neighbor, int width)
    {
        Direction dir = Direction::North;
        if(current.X != neighbor.X) {
            assert(current.Y == neighbor.Y);
            dir = Gorgon::Sign(neighbor.X - current.X) > 0 ? Direction::East : Direction::West;
        }
        else {
            assert(current.X == neighbor.X);
            dir = Gorgon::Sign(neighbor.Y - current.Y) > 0 ? Direction::South : Direction::North;
        }
        map.push_back({current, dir});
    }

    std::vector<std::pair<Point, Direction>> RecursiveBacktracker(int width, int height) {
        std::vector<std::pair<Point, Direction>> map;
        std::vector<int> visited(width * height, 0);
        std::vector<Point> tmp;
        int startx = std::rand() % width;
        int starty = std::rand() % height;
        tmp.push_back({startx, starty});
        visited[Get1DCoordinate({startx, starty}, width)] = 1;
        while(!tmp.empty()) {
            auto curr = tmp.back();
            tmp.pop_back();
            auto unvisited = FindUnvisitedNeighbors(visited, curr, width, height);
            if(unvisited.size() > 0) {
                tmp.push_back(curr);
                auto neighbor = unvisited[std::rand() % unvisited.size()];
                assert(neighbor.X >= 0 && neighbor.X < width && neighbor.Y >= 0 && neighbor.Y < height);
                RemoveWall(map, curr, neighbor, width);
                visited[Get1DCoordinate(neighbor, width)] = 1;
                tmp.push_back(neighbor);
            }
        }
        return map;
    }
}

