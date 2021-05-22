#include "Map.h"
#include <Gorgon/Geometry/PointList.h>

Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tileproviders;
Gorgon::Containers::Collection<Gorgon::Graphics::RectangularAnimation> tiles;

Gorgon::Geometry::PointList<Point> points = {
    {2, 4}, {2, 10}, {15, 10}, {15, 5}, {18, 5}, {18, 18}
};

Map::Map() {
    if(tileproviders.GetSize() == 0) {
        std::string path = "../resource/Kenney TD/Default size/";
        Gorgon::Graphics::Bitmap &first = tileproviders.AddNew();
        first.Import(path + "towerDefense_tile024.png");
        first.Prepare();
        tiles.Add(first);
        
        auto &second = tileproviders.AddNew();
        second.Import(path + "towerDefense_tile050.png");
        second.Prepare();
        tiles.Add(second);
        
        auto &third = tileproviders.AddNew();
        third.Import(path + "towerDefense_tile001.png");
        third.Prepare();
        tiles.Add(third);
        
        auto &four = tileproviders.AddNew();
        four.Import(path + "towerDefense_tile047.png");
        four.Prepare();
        tiles.Add(four);
    }
    
    mapsize = {20, 20};
    std::fill_n(std::back_inserter(map), 400, 0);
    
    Point cur = points[0];
    for(int i=1; i<points.GetSize(); i++) {
        if(cur.X != points[i].X) {
            int dir = Gorgon::Sign(points[i].X - cur.X);
            for(int x=cur.X; x!=points[i].X; x+=dir) {
                Set(x, cur.Y, 1);
                Set(x, cur.Y+1, 2);
                Set(x, cur.Y-1, 3);
            }
        }
        else {
            int dir = Gorgon::Sign(points[i].Y - cur.Y);
            for(int y=cur.Y; y!=points[i].Y; y+=dir) {
                Set(cur.X, y, 1);
            }
        }
        
        cur = points[i];
    }
}


void Map::Render(Gorgon::Graphics::Layer &target) {
    for(int y=0; y<mapsize.Height; y++) {
        for(int x=0; x<mapsize.Width; x++) {
            tiles[(*this)(x, y)].DrawStretched(target, Point(x*tilesize.Width, y*tilesize.Height), tilesize);
        }
    }
}
