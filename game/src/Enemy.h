#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>

#include "Types.h"

class EnemyType {
    friend bool LoadResources();
public:
    
    static std::map<std::string, EnemyType> Enemies;
    
    
protected:
    std::string id;
    std::string name;
    EnemyClass type;
    float speed;
    int hitpoints;
    int armor;
    int reactivearmor;
    int shield;
    float evasion;
    int scraps;
    int strength;
    Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> image;

};
