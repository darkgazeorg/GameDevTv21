#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Geometry/PointList.h>

#include "Types.h"

extern const Size EnemySize;

class EnemyType {
    friend bool LoadResources();
    friend class Wave;
    friend class Enemy;
public:
    
    static Gorgon::Containers::Collection<EnemyType> Enemies;
    
    void Print(Gorgon::Graphics::Layer &target, Point location, int width, int count = 0);
    
    
    void RenderIcon(Gorgon::Graphics::Layer &target, Point location);
    
    Sizef GetSize() const {
        return Sizef(image[0].GetSize()) / EnemySize;
    }
    
    float GetSpeed() const {
        return speed;
    }
    
    EnemyClass GetType() const {
        return type;
    }
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
    Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> shadow;
};


class EnemyGroup {
public:
    EnemyType *enemy;
    int count;
    int inrow;
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
        base(&enemy),
        groupind(groupind),
        path(&path)
    { 
        hpleft = base->hitpoints;
        auto sz = base->GetSize();
        size = (sz.Width+sz.Height) / 4.f;
    }
    
    //offset is the offset of the map
    void Render(Gorgon::Graphics::Layer &target, Point offset, Size tilesize);
    
    //returns the damage if the enemy reaches to the end. Otherwise returns 0.
    int Progress(int delta);
    
    float GetSize() const {
        return size;
    }
    
    Pointf GetLocation() const {
        auto st = (*path)[locationpoint];
        auto ed = path->GetLine(locationpoint).End;
        Pointf pnt = (st * (1-offsetfrompoint) + ed * offsetfrompoint);
        return pnt;
    }
    
    int GetScraps() const {
        return base->scraps;
    }
    
    bool ApplyDamage(int damage, DamageType type);
    
    EnemyClass GetType() const {
        return base->type;
    }
    
private:
    const EnemyType *base;
    int groupind;
    const Gorgon::Geometry::PointList<> *path;
    int locationpoint   = 0;
    float offsetfrompoint = 0;
    float hpleft;
    float size;
};
