#include "ImProc.h"
#include "Types.h"
#include <math.h>


Gorgon::Graphics::Bitmap Rotate(const Gorgon::Graphics::Bitmap &bmp, float angle) {
    Gorgon::Graphics::Bitmap ret(bmp.GetSize(), bmp.GetMode());
    
    Pointf offset = Pointf(Sizef(bmp.GetSize()) / 2.f);
    int C = bmp.GetChannelsPerPixel();
    
    ret.ForAllPixels([&](int x, int y) {
        auto t = Pointf(x, y);
        Rotate(t, angle, offset);
        
        float fx = t.X - floor(t.X);
        float fy = t.Y - floor(t.Y);
        float mfx = 1 - fx;
        float mfy = 1 - fy;
        
        int tx = int(floor(t.X));
        int ty = int(floor(t.Y));
        
        for(int c=0; c<C; c++)
            ret(x, y, c) = mfx * mfy * bmp.Get({tx  , ty  }, c) + 
                            fx * mfy * bmp.Get({tx+1, ty  }, c) + 
                           mfx *  fy * bmp.Get({tx  , ty+1}, c) + 
                            fx *  fy * bmp.Get({tx+1, ty+1}, c);
    });
    
    ret.Prepare();
    return ret;
}

Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> CreateRotations(
    const Gorgon::Graphics::Bitmap &bmp, int rotations
) {
    Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> ret;
    
    float ang = Gorgon::PI * 2.f / rotations;
    for(int i=0; i<rotations; i++) {
        ret.AddNew(Rotate(bmp, i*ang));
    }
    
    return ret;
}
