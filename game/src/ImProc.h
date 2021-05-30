#pragma once

#include <Gorgon/Containers/Collection.h>
#include <Gorgon/Graphics/Bitmap.h>

#include "Types.h"

//Uses bilinear interpolation. Angle is in radians. Does not change image size. Uses image center as
//pivot point
Gorgon::Graphics::Bitmap Rotate(const Gorgon::Graphics::Bitmap &bmp, float angle);

Gorgon::Graphics::Bitmap Scale(const Gorgon::Graphics::Bitmap &bmp, Size newsize);

Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> CreateRotations(
    const Gorgon::Graphics::Bitmap &bmp, int rotations
);
