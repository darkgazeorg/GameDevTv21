#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Geometry/PointList.h>

#include "Types.h"

class EnemyType {
    friend bool LoadResources();
    friend class Wave;
    friend class Enemy;
public:
    
    static Gorgon::Containers::Collection<EnemyType> Enemies;
    
    void Print(Gorgon::Graphics::Layer &target, Point location, int width, int count = 0);
    
    
    void RenderIcon(Gorgon::Graphics::Layer &target, Point location);
    
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


class EnemyGroup {
public:
    EnemyType *enemy;
    int count;
    float delay;
};

class Wave {
public:
    
    Wave() { }
    
    Wave(int totalstrength, std::default_random_engine &random);
    
    
    
    std::vector<EnemyGroup> Enemies;
    
};


class Enemy {
public:
    Enemy(const EnemyType &enemy, int groupind, const Gorgon::Geometry::PointList<> &path) :
        base(enemy),
        groupind(groupind),
        path(path)
    { }
    
    //offset is the offset of the map
    void Render(Gorgon::Graphics::Layer &target, Point offset, Size tilesize);
    
    //returns the damage if the enemy reaches to the end. Otherwise returns 0.
    int Progress(int delta);
    
private:
    const EnemyType &base;
    int groupind;
    const Gorgon::Geometry::PointList<> &path;
    int locationpoint   = 0;
    float offsetfrompoint = 0;
};
