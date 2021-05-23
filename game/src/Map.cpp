#include "Map.h"
#include <Gorgon/Geometry/PointList.h>
#include <Gorgon/Resource/File.h>
#include <Gorgon/Resource/Image.h>
#include <cstdint>
#include <cstdlib>
#include <cassert>

namespace R = Gorgon::Resource;

R::File resources;
Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tilesets;
std::vector<Gorgon::Graphics::TextureImage> tiles;

Gorgon::Geometry::PointList<Point> points = {
    {1, 2}, {6, 2}, {6, 4}, {12, 4}, {12, 10}, {6, 10}, {6, 8}, {2, 8}, {2, 16}, {8, 16}, {8, 14}, {18, 14}, {18, 10}
};

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
            assert(false);
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
            assert(false);
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


    TileIndex corners[8][3][3] = {
    {
        {TileIndex::Max, TileIndex::Bottom, TileIndex::BottomLeft},
        {TileIndex::Full, TileIndex::Full, TileIndex::Left},
        {TileIndex::InnerBottomLeft, TileIndex::Full, TileIndex::Left},
    },
    {
        {TileIndex::Max, TileIndex::Full, TileIndex::InnerTopRight},
        {TileIndex::Right, TileIndex::Full, TileIndex::Full},
        {TileIndex::TopRight, TileIndex::Top, TileIndex::Top},
    },
    {
        {TileIndex::BottomRight, TileIndex::Bottom, TileIndex::Max},
        {TileIndex::Right, TileIndex::Full, TileIndex::Full},
        {TileIndex::Right, TileIndex::Full, TileIndex::InnerBottomRight},
    },
    {
        {TileIndex::BottomRight, TileIndex::Bottom, TileIndex::Bottom},
        {TileIndex::Right, TileIndex::Full, TileIndex::Full},
        {TileIndex::Max, TileIndex::Full, TileIndex::InnerBottomRight},
    },
    {
        {TileIndex::InnerTopLeft, TileIndex::Full, TileIndex::Left},
        {TileIndex::Full, TileIndex::Full, TileIndex::Left},
        {TileIndex::Max, TileIndex::Top, TileIndex::TopLeft},
    },
    {
        {TileIndex::InnerTopLeft, TileIndex::Full, TileIndex::Max},
        {TileIndex::Full, TileIndex::Full, TileIndex::Left},
        {TileIndex::Top, TileIndex::Top, TileIndex::TopLeft},
    },
    {
        {TileIndex::Right, TileIndex::Full, TileIndex::InnerTopRight},
        {TileIndex::Right, TileIndex::Full, TileIndex::Full},
        {TileIndex::TopRight, TileIndex::Top, TileIndex::Max},
    },
    {
        {TileIndex::Bottom, TileIndex::Bottom, TileIndex::BottomLeft},
        {TileIndex::Full, TileIndex::Full, TileIndex::Left},
        {TileIndex::InnerBottomLeft, TileIndex::Full, TileIndex::Max},
    },
};

Map::Map(std::default_random_engine &random) {
    if(resources.Root().GetCount() == 0) {
        resources.LoadFile("Resources_1x.gor");
        resources.Prepare();
        
        auto &tilesfold = resources.Root().Get<R::Folder>(0);
        
        for(auto &res : tilesfold) {
            if(res.GetGID() == R::GID::Image) {
                auto &im = dynamic_cast<R::Image&>(res);
                tilesets.AddNew(im.MoveOutAsBitmap());
            }
        }
    }
    
    auto &tileset = tilesets[std::uniform_int_distribution<int>(0, tilesets.GetSize()-1)(random)];
    auto size = tileset.GetHeight()/3;
    
    Gorgon::Geometry::PointList<Point> tilelocations = {
        {1, 1}, {4, 2}, {3, 2}, //empty, full, single
        {0, 1}, {2, 1},         //left, right
        {1, 0}, {1, 2},         //top, bottom,
        //corners are inverted!
        {3, 0}, {4, 0},         //topleft, topright
        {3, 1}, {4, 1},         //bottomleft, bottomright
        //inner
        {0, 0}, {2, 0},         //topleft, topright
        {0, 2}, {2, 2},         //bottomleft, bottomright
    };
    
    Scale(tilelocations, size);
    
    std::vector<Gorgon::Geometry::Bounds> tilebounds;
    for(auto p : tilelocations) {
        tilebounds.push_back({p, size, size});
    }
    tiles = tileset.CreateAtlasImages(tilebounds);
    
    auto maze = RecursiveBacktracker(10, 10);
    mapsize = {20, 20};
    
    std::fill_n(std::back_inserter(map), mapsize.Area(), 0);
    
    Point cur = points[0];
    int pdir = 0;
    for(int i=1; i<points.GetSize(); i++) {
        int dir;
        if(cur.X != points[i].X) {
            dir = Gorgon::Sign(points[i].X - cur.X);
            for(int x=cur.X; x!=points[i].X; x+=dir) {
                Set(x, cur.Y, TileIndex::Full);
                if((x != cur.X && x != cur.X+dir) || pdir == 0) {
                    Set(x, cur.Y+1, TileIndex::Top);
                    Set(x, cur.Y-1, TileIndex::Bottom);
                }
            }
        }
        else {
            dir = Gorgon::Sign(points[i].Y - cur.Y);
            for(int y=cur.Y; y!=points[i].Y; y+=dir) {
                Set(cur.X, y, TileIndex::Full);
                
                if((y != cur.Y && y != cur.Y+dir) || pdir == 0) {
                    Set(cur.X+1, y, TileIndex::Left);
                    Set(cur.X-1, y, TileIndex::Right);
                }
            }
        }
        
        if(i != points.GetSize() - 1) {
            int templ = 0;
            
            bool curyaxis  = cur.Y != points[i].Y;
            bool nextyaxis = !curyaxis;
            int  nextdir    = nextyaxis ? 
                Gorgon::Sign(points[i+1].Y - points[i].Y) : 
                Gorgon::Sign(points[i+1].X - points[i].X)
            ;
            auto next = points[i];
            
            templ = curyaxis | (dir == -1) << 1 | (nextdir == -1) << 2;
            
            for(int i=0; i<3; i++)
                for(int j=0; j<3; j++)
                    if(corners[templ][i][j] != TileIndex::Max)
                        Set(next.X+j-1, next.Y+i-1, corners[templ][i][j]);
        }
        
        cur = points[i];
        pdir = dir;
    }

}

void Map::Render(Gorgon::Graphics::Layer &target) {
    for(int y=0; y<mapsize.Height; y++) {
        for(int x=0; x<mapsize.Width; x++) {
            tiles[(*this)(x, y)].DrawStretched(target, Point(x*tilesize.Width, y*tilesize.Height), tilesize);
        }
    }
}
