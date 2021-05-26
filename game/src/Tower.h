#pragma once

#include <string>
#include <vector>

#include "Types.h"

class TowerType {
public:
    
    
private:
    std::string id;
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
    std::vector<Point> bulletlocations;
    bool displaybullets;
};
