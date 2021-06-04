#pragma once

#include "Types.h"

#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/CGI/Bezier.h>

enum class TileIndex {
    Empty, 
    Full, 
    Single,
    Left, 
    Right,
    Top, 
    Bottom,
    BottomRight,
    BottomLeft, 
    TopRight,
    TopLeft, 
    InnerBottomRight,
    InnerBottomLeft,
    InnerTopRight, 
    InnerTopLeft,
    Building1,
    Building2,
    Building3,
    Building4,
    Max
};

class Map {
public:
    Map(std::default_random_engine &random);
    
    void Render(Gorgon::Graphics::Layer &target);
    
    int &operator()(int x, int y) {
        return map[x + y*mapsize.Width];
    }
    
    const int &operator()(int x, int y) const {
        return map[x + y*mapsize.Width];
    }
    
    void Set(int x, int y, TileIndex val) {
        if(x < 0 || x >= mapsize.Width || y < 0 || y >= mapsize.Height)
            return;
        
        map[x + y*mapsize.Width] = (int)val;
    }
    
    Size GetSize() const {
        return mapsize;
    }
    
    Point offset = {0, 0};
    Gorgon::Containers::Collection<Gorgon::Geometry::PointList<>> Paths;
    
protected:
    std::vector<int> map;
    Size mapsize;
    Size tilesize = {48, 48};
    Size gridsize = {1, 1};
    std::vector<Gorgon::Graphics::TextureImage> &tileset;
    Gorgon::Graphics::Bitmap debug;
};
