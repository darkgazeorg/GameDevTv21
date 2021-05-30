#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>

#include "Types.h"

class EnemyType {
    friend bool LoadResources();
    friend class Wave;
public:
    
    static Gorgon::Containers::Collection<EnemyType> Enemies;
    
    
//protected:
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

class EnemyGroup {
public:
    EnemyType *enemy;
    int count;
    float delay;
};

class Wave {
public:
    
    Wave(int totalstrength, std::default_random_engine &random);
    
    
    
    std::vector<EnemyGroup> Enemies;
    
};
