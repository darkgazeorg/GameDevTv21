#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Resource/AnimationStorage.h>

#include "Types.h"

class Enemy;

class TowerType {
    friend bool LoadResources();
    friend class Tower;
public:
    
    static std::map<std::string, TowerType> Towers;
    
    void Print(Gorgon::Graphics::Layer &target, Point location, int width, bool highlight, bool disabled) const;
    
    void RenderIcon(Gorgon::Graphics::Layer &target, Point location) const;
    
    bool IsPlacable() const {
        return placable;
    }
    
    int GetCost() const {
        return cost;
    }
    
    const std::vector<std::string> GetUpgrades() const {
        return upgradesto;
    }
    
    void DrawRange(Gorgon::Graphics::Layer &target, Point offset, Size tilesize);
    
private:
    std::string id;
    std::string name;
    int damageperbullet;
    float reloadtime;
    int numberofbullets;
    bool continuousreload;
    float bulletspeed;
    float bulletacceleration;
    DamageType damagetype;
    float areasize;
    float distancefalloff;
    float areafalloff;
    int cost;
    std::vector<std::string> upgradesto;
    float range;
    bool placable;
    TargetType target;
    EnemyClass effectiveagainst;
    float effectivemultiplier;
    std::vector<Pointf> bulletlocations;
    bool displaybullets;
    Gorgon::Graphics::Bitmap *base = nullptr;
    Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> top;
    Gorgon::Graphics::Bitmap *effect = nullptr;
    Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> bullet;
    Gorgon::Graphics::Bitmap *bulleteffect = nullptr;
    Gorgon::Graphics::Bitmap rangeimage;
};

class Bullet {
public:
    long int target;
    Pointf location;
    int angle;
    bool done;
    Pointf start;
};

class Tower {
public:
    Tower(const TowerType &base, Point location, bool instant) : 
        base(&base),
        location(location)
    {
        currentbullets = base.numberofbullets;
        if(!instant)
            construction   = std::min(std::max((int)std::round(base.cost * 100 + log(base.cost)*1000), 5000), 30000);
    }
    
    void Render(Gorgon::Graphics::Layer &target, Point offset, Size tilesize, bool showrange);
    
    //returns scraps
    int Progress(unsigned delta, std::map<long int, Enemy> &enemies);
    
    void Print(Gorgon::Graphics::Layer &target, Point location, int width);
    
    Point GetLocation() const {
        return location;
    }
    
    const TowerType &GetType() const {
        return *base;
    }
    
    bool UnderConstruction() const {
        return construction != 0;
    }
    
private:
    const TowerType *base;
    Point  location;
    long int tracktarget = -1;
    int currentbullets;
    std::vector<Bullet> flyingbullets;
    std::vector<std::pair<Pointf, float>> explosions; //second is timer
    int angle = 0;
    int construction = 0;
    int reloadloop = 0;
    int nextfire = 0;
    int kills = 0;
    int damage = 0;
};
