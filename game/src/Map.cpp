#include "ImProc.h"
#include "Map.h"
#include "MapGen.h"
#include "Resources.h"
#include "Types.h"

#include <Gorgon/CGI/Line.h>
#include <Gorgon/Geometry/PointList.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace {
    Size getsize(const std::vector<Point>& path) {
        const auto findminx = [] (const Point& lhs, const Point& rhs) { return lhs.X < rhs.X; };
        const auto findmaxx = [] (const Point& lhs, const Point& rhs) { return lhs.X > rhs.X; };
        const auto findminy = [] (const Point& lhs, const Point& rhs) { return lhs.Y < rhs.Y; };
        const auto findmaxy = [] (const Point& lhs, const Point& rhs) { return lhs.Y > rhs.Y; };
        int minx = (*std::min_element(path.begin(), path.end(), findminx)).X;
        int maxx = (*std::min_element(path.begin(), path.end(), findmaxx)).X;
        int miny = (*std::min_element(path.begin(), path.end(), findminy)).Y;
        int maxy = (*std::min_element(path.begin(), path.end(), findmaxy)).Y;
        return Size(maxx - minx + 1, maxy - miny + 1);
    }
}

Gorgon::Geometry::PointList<Point> points = {
    {1, 2}, {6, 2}, {6, 4}, {12, 4}, {12, 10}, {6, 10}, {6, 8}, {3, 8}, {3, 16}, {8, 16}, {8, 14}, {18, 14}, {18, 10}
};

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

Map::Map(std::default_random_engine &random) 
    : tileset(tiles[std::uniform_int_distribution<int>(0, ::tilesets.GetSize()-1)(random)])
{
    int seed = std::uniform_int_distribution<int>(0, 32000)(random);
    srand(seed);
    mapsize = {33, 21};
    
    std::fill_n(std::back_inserter(map), mapsize.Area(), 0);

    auto checklength = [] (const std::vector<Point>& path) { return path.size() > 8 && path.size() <= 30; };
    PathChecker checker({checklength});
    Size mazesize(7, 5);
    RecursiveBacktracker mazegen;

    std::vector<std::vector<Point>> solutions;
    while(solutions.size() < 100) {
        auto solution = mazegen.Solve(mazegen.Generate(mazesize), mazesize);
        if(checker.Check(solution)) {
            solutions.push_back(solution);
        }
    }

    constexpr int cellscale = 3;
    int xoffset = 0;
    int yoffset = 0;
    std::vector<std::vector<Point>> newsolutions;
    std::vector<std::pair<int, int>> lengths;
    int ind = 0;
    for(auto& solution: solutions) {
        auto newsol = StretchUTurns(solution);
        float distance = solution.front().Distance(solution.back());
        Size pathsize(getsize(newsol) * cellscale);
        if(mapsize.Width > pathsize.Width && mapsize.Height > pathsize.Height && distance >= 5) {
            newsolutions.push_back(newsol);
            int len = 0;
            for(int i=1; i<newsol.size(); i++) {
                len += newsol[i].ManhattanDistance(newsol[i-1]);
            }
            lengths.push_back({ind, len});
            ind++;
        }
    }
    ASSERT(newsolutions.size() > 0, "failed to generate a path");

    std::sort(lengths.begin(), lengths.end(), [] (auto &l, auto &r) {
        return std::abs(l.second - 30) < std::abs(r.second - 30);
    });

    auto solution = newsolutions[lengths[0].first];
    Size pathsize(getsize(solution) * cellscale);
    solution = ConnectEnteranceToEdge(solution, pathsize / cellscale, mapsize / cellscale);
    xoffset = (mapsize.Width - pathsize.Width) / 2 + 1;
    yoffset = (mapsize.Height - pathsize.Height) / 2 + 1;
    Gorgon::Geometry::PointList<Point> points;
    for(auto point : solution) {
        points.Push({point.X * cellscale + xoffset, point.Y * cellscale + yoffset});
    }

    //flatten point list
    for(int i=1; i<points.GetSize()-1; i++) {
        if(points[i-1].X == points[i].X && points[i].X == points[i+1].X) {
            points.Points.erase(points.begin() + i);
            i--;
        }
        else if(points[i-1].Y == points[i].Y && points[i].Y == points[i+1].Y) {
            points.Points.erase(points.begin() + i);
            i--;
        }
    }

    //create tile map
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
    
    int total = 0;
    for(int i=0; i<mapsize.Area(); i++) {
        total += map[i] != 0;
    }
    
    std::cout << "Path area: " << total << std::endl;
    
    //create paths
    
    //starting points
    std::vector<Gorgon::CGI::Curves> curves;
    curves.resize(9);
    int dir;
    
    bool nextyaxis = points[1].X != points[0].X;
    int  nextdir    = !nextyaxis ? 
        Gorgon::Sign(points[1].Y - points[0].Y) : 
        Gorgon::Sign(points[1].X - points[0].X)
    ;
    
    if(points[1].X != points[0].X) {
        for(int y=-4; y<=4; y++) {
            curves[y+4].SetStartingPoint(
                {points[0].X+0.5f - nextdir, points[0].Y + 0.5f + y/4.f*nextdir}
            );
        }
    }
    else {
        for(int x=-4; x<=4; x++) {
            curves[x+4].SetStartingPoint(
                {points[0].X + 0.5f - x/4.f*nextdir, points[0].Y - nextdir + 0.5f}
            );
        }
    }
    
    cur = points[0];
    for(int i=1; i<points.GetSize(); i++) {
        if(cur.X != points[i].X) {
            dir = Gorgon::Sign(points[i].X - cur.X);
            for(int y=-4; y<=4; y++) {
                curves[y+4].Push({points[i].X+0.5f - 1*dir, points[i].Y + 0.5f + y/4.f*dir});
            }
        }
        else {
            dir = Gorgon::Sign(points[i].Y - cur.Y);
            for(int x=-4; x<=4; x++) {
                curves[x+4].Push({points[i].X + 0.5f - x/4.f*dir, points[i].Y + 0.5f - 1*dir});
            }
        }
        
        if(i != points.GetSize() - 1) {
            bool curyaxis  = cur.Y != points[i].Y;
            bool nextyaxis = !curyaxis;
            int  nextdir    = nextyaxis ? 
                Gorgon::Sign(points[i+1].Y - points[i].Y) : 
                Gorgon::Sign(points[i+1].X - points[i].X)
            ;
            
            if(points[i+1].X != points[i].X) {
                for(int y=-4; y<=4; y++) {
                    curves[y+4].Push(
                        {curves[y+4].Get( curves[y+4].GetCount()-1).P3.X, points[i].Y + 0.5f + y/4.f*nextdir},
                        {points[i].X+0.5f + nextdir, points[i].Y + 0.5f + y/4.f*nextdir}
                    );
                }
            }
            else {
                for(int x=-4; x<=4; x++) {
                    curves[x+4].Push(
                        {points[i].X + 0.5f - x/4.f*nextdir, curves[x+4].Get( curves[x+4].GetCount()-1).P3.Y},
                        {points[i].X + 0.5f - x/4.f*nextdir, points[i].Y + nextdir + 0.5f}
                    );
                }
            }
        }
        
        cur = points[i];
    }
    
    int last = points.GetSize() - 1;
    if(points[last-1].X != points[last].X) {
        dir = Gorgon::Sign(points[last].X - points[last-1].X);
        for(int y=-4; y<=4; y++) {
            curves[y+4].Push({points[last].X+0.5f, points[last].Y + 0.5f + y/4.f*dir});
        }
    }
    else {
        dir = Gorgon::Sign(points[last].Y - points[last-1].Y);
        for(int x=-4; x<=4; x++) {
            curves[x+4].Push({points[last].X + 0.5f - x/4.f*dir, points[last].Y+0.5f});
        }
    }
    
    //flatten
    Paths.Destroy();
    for(auto &curve : curves) {
        Paths.AddNew(curve.Flatten(0.05));
    }
    Paths.AddNew(Gorgon::Geometry::PointList<>{Paths[0].Front(), Paths[0].Back()});

    debug.Resize(mapsize * tilesize);
    debug.Clear();
    
    for(int y=0; y<mapsize.Height; y++) {
        Gorgon::CGI::DrawLines(debug, {{0, y*tilesize.Height+0.5f}, {(float)debug.GetWidth(), y*tilesize.Height+0.5f}}, gridsize.Width, Gorgon::CGI::SolidFill<>({Color::Grey, 0.5f}));
    }
    for(int x=0; x<mapsize.Width; x++) {
        Gorgon::CGI::DrawLines(debug, {{x*tilesize.Width+0.5f, 0}, {x*tilesize.Width+0.5f, (float)debug.GetHeight()}}, gridsize.Height, Gorgon::CGI::SolidFill<>({Color::Grey, 0.5f}));
    }
    for(auto &path : curves ) {
        Gorgon::CGI::DrawLines(debug, path.Flatten(0.05f)*Sizef(tilesize), 0.5f, Gorgon::CGI::SolidFill<>({Color::Aqua}));
    }
    
    debug.Prepare();
}

void Map::Render(Gorgon::Graphics::Layer &target) {
    offset = Point((target.GetTargetSize() - (tilesize * mapsize))/2);
    for(int y=0; y<mapsize.Height; y++) {
        for(int x=0; x<mapsize.Width; x++) {
            tileset[(*this)(x, y)].DrawStretched(target, Point(x*tilesize.Width, y*tilesize.Height)+offset, tilesize);
        }
    }
    debug.Draw(target, offset);
}
