#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Resource/AnimationStorage.h>

#include "Types.h"

class TowerType {
    friend bool LoadResources();
public:
    
    static std::map<std::string, TowerType> Towers;
    
    void Print(Gorgon::Graphics::Layer &target, Point location, int width, bool highlight, bool disabled);
    
    void RenderIcon(Gorgon::Graphics::Layer &target, Point location);
    
    bool IsPlacable() const {
        return placable;
    }
    
    int GetCost() const {
        return cost;
    }
    
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
};
