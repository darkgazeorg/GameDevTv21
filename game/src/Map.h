#pragma once

#include "Types.h"

#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>

class Map {
public:
    Map();
    
    void Render(Gorgon::Graphics::Layer &target);
    
    int &operator()(int x, int y) {
        return map[x + y*mapsize.Width];
    }
    
    const int &operator()(int x, int y) const {
        return map[x + y*mapsize.Width];
    }
    
    void Set(int x, int y, int val) {
        if(x < 0 || x >= mapsize.Width || y < 0 || y >= mapsize.Height)
            return;
        
        map[x + y*mapsize.Width] = val;
    }
    
protected:
    std::vector<int> map;
    Size mapsize;
    Size tilesize = {32, 32};
};
