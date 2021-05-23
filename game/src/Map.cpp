#include "Map.h"
#include <Gorgon/Geometry/PointList.h>
#include <Gorgon/Resource/File.h>
#include <Gorgon/Resource/Image.h>

namespace R = Gorgon::Resource;

R::File resources;
Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tilesets;
std::vector<Gorgon::Graphics::TextureImage> tiles;

Gorgon::Geometry::PointList<Point> points = {
    {1, 2}, {6, 2}, {6, 4}, {12, 4}, {12, 10}, {6, 10}, {6, 8}, {2, 8}, {2, 16}, {8, 16}, {8, 14}, {18, 14}, {18, 10}
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
