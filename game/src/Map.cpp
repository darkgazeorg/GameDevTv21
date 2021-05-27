#include "Map.h"
#include "MapGen.h"
#include "Types.h"
#include <Gorgon/Geometry/PointList.h>
#include <Gorgon/Resource/File.h>
#include <Gorgon/Resource/Image.h>
#include <Gorgon/CGI/Line.h>
#include <cstdint>
#include <cstdlib>
#include <cassert>

namespace R = Gorgon::Resource;

R::File resources;
Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tilesets;
std::vector<Gorgon::Graphics::TextureImage> tiles;

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
    
    mapsize = {33, 21};
    
    std::fill_n(std::back_inserter(map), mapsize.Area(), 0);

    
    Size mazesize(7, 5);
    RecursiveBacktracker mazegen;
    auto maze = mazegen.Generate(mazesize);
    auto solution = mazegen.Solve(maze, mazesize);
    Gorgon::Geometry::PointList<Point> points;
    for(auto point : solution) {
        points.Push({point.X * 3+6, point.Y * 3+6});
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
    
    //create paths
    
    //starting points
    paths.resize(9);
    int dir;
    
    bool nextyaxis = points[1].X != points[0].X;
    int  nextdir    = !nextyaxis ? 
        Gorgon::Sign(points[1].Y - points[0].Y) : 
        Gorgon::Sign(points[1].X - points[0].X)
    ;
    
    if(points[1].X != points[0].X) {
        for(int y=-4; y<=4; y++) {
            paths[y+4].SetStartingPoint(
                {points[0].X+0.5 - nextdir, points[0].Y + 0.5 + y/4.f*nextdir}
            );
        }
    }
    else {
        for(int x=-4; x<=4; x++) {
            paths[x+4].SetStartingPoint(
                {points[0].X + 0.5 - x/4.f*nextdir, points[0].Y - nextdir + 0.5}
            );
        }
    }
    
    cur = points[0];
    for(int i=1; i<points.GetSize(); i++) {
        if(cur.X != points[i].X) {
            dir = Gorgon::Sign(points[i].X - cur.X);
            for(int y=-4; y<=4; y++) {
                paths[y+4].Push({points[i].X+0.5 - 1*dir, points[i].Y + 0.5 + y/4.f*dir});
            }
        }
        else {
            dir = Gorgon::Sign(points[i].Y - cur.Y);
            for(int x=-4; x<=4; x++) {
                paths[x+4].Push({points[i].X + 0.5 - x/4.f*dir, points[i].Y + 0.5 - 1*dir});
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
                    paths[y+4].Push(
                        {paths[y+4].Get(paths[y+4].GetCount()-1).P3.X, points[i].Y + 0.5 + y/4.f*nextdir},
                        {points[i].X+0.5 + nextdir, points[i].Y + 0.5 + y/4.f*nextdir}
                    );
                }
            }
            else {
                for(int x=-4; x<=4; x++) {
                    paths[x+4].Push(
                        {points[i].X + 0.5 - x/4.f*nextdir, paths[x+4].Get(paths[x+4].GetCount()-1).P3.Y},
                        {points[i].X + 0.5 - x/4.f*nextdir, points[i].Y + nextdir + 0.5}
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
            paths[y+4].Push({points[last].X+0.5, points[last].Y + 0.5 + y/4.f*dir});
        }
    }
    else {
        dir = Gorgon::Sign(points[last].Y - points[last-1].Y);
        for(int x=-4; x<=4; x++) {
            paths[x+4].Push({points[last].X + 0.5 - x/4.f*dir, points[last].Y+0.5});
        }
    }

    debug.Resize(mapsize * tilesize);
    debug.Clear();
    
    for(int y=0; y<mapsize.Height; y++) {
        Gorgon::CGI::DrawLines(debug, {{0, y*tilesize.Height+0.5}, {debug.GetWidth(), y*tilesize.Height+0.5}}, gridsize.Width, Gorgon::CGI::SolidFill<>({Color::Grey, 0.5}));
    }
    for(int x=0; x<mapsize.Width; x++) {
        Gorgon::CGI::DrawLines(debug, {{x*tilesize.Width+0.5, 0}, {x*tilesize.Width+0.5, debug.GetHeight()}}, gridsize.Height, Gorgon::CGI::SolidFill<>({Color::Grey, 0.5}));
    }
    for(auto &path : paths) {
        Gorgon::CGI::DrawLines(debug, path.Flatten(0.05)*Sizef(tilesize), 2, Gorgon::CGI::SolidFill<>({Color::Aqua}));
    }
    
    debug.Prepare();
}

void Map::Render(Gorgon::Graphics::Layer &target) {
    Point offset = Point((target.GetTargetSize() - (tilesize * mapsize))/2);
    for(int y=0; y<mapsize.Height; y++) {
        for(int x=0; x<mapsize.Width; x++) {
            tiles[(*this)(x, y)].DrawStretched(target, Point(x*tilesize.Width, y*tilesize.Height)+offset, tilesize);
        }
    }
    debug.Draw(target, offset);
}
