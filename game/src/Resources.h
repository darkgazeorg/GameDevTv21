#pragma once

#include "Types.h"

#include <Gorgon/Graphics/Bitmap.h>

extern R::File resources;

extern Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tilesets;
extern std::vector<std::vector<Gorgon::Graphics::TextureImage>> tiles;

bool LoadResources();
