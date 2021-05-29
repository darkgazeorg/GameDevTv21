#include "Resources.h"

#include <Gorgon/Geometry/PointList.h>
#include <Gorgon/Resource/Image.h>

R::File resources;

Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tilesets;
std::vector<std::vector<Gorgon::Graphics::TextureImage>> tiles;

bool LoadResources() {
    if(resources.Root().GetCount() != 0)
        return true;
    
    resources.LoadFile("Resources_1x.gor");
    resources.Prepare();
    
    auto &tilesfold = resources.Root().Get<R::Folder>(0);
    
    for(auto &res : tilesfold) {
        if(res.GetGID() == R::GID::Image) {
            auto &im = dynamic_cast<R::Image&>(res);
            tilesets.AddNew(im.MoveOutAsBitmap());
        }
    }
    
    for(auto &tileset : tilesets) {
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
        tiles.push_back(tileset.CreateAtlasImages(tilebounds));
    }
    
    return true;
}
