#pragma once

#include <string>
#include <vector>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Resource/AnimationStorage.h>

#include "Types.h"

class TowerType {
    friend bool LoadResources();
public:
    
    static std::map<std::string, TowerType> Towers;
    
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
    EnemyType effectiveagainst;
    float effectivemultiplier;
    std::vector<Pointf> bulletlocations;
    bool displaybullets;
    R::AnimationStorage *base = nullptr;
    R::AnimationStorage *top = nullptr;
    R::AnimationStorage *effect = nullptr;
    R::AnimationStorage *bullet = nullptr;
    R::AnimationStorage *bulleteffect = nullptr;
};
