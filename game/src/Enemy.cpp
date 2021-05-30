#include "Enemy.h"

Gorgon::Containers::Collection<EnemyType> EnemyType::Enemies;

float randfloat(std::default_random_engine &random, float min = 0, float max = 1) {
    return std::uniform_real_distribution<float>(min, max)(random);
}

int randint(std::default_random_engine &random, int min, int max) {
    return std::uniform_int_distribution<int>(min, max)(random);
}

Wave::Wave(int ts, std::default_random_engine &random) {
    int minstr = std::numeric_limits<int>::max(), maxstr = 0;
    float avg = 0;
    Gorgon::Containers::Collection<EnemyType> soloenemies;
    for(auto &enemy : EnemyType::Enemies) {
        if(minstr > enemy.strength)
            minstr = enemy.strength;
        if(maxstr < enemy.strength)
            maxstr = enemy.strength;
        avg += enemy.strength;
    }
    
    avg /= EnemyType::Enemies.GetSize();
    
    int sololimit = 1000;//int((avg + maxstr*2) / 3);

    for(auto &enemy : EnemyType::Enemies) {
        if(enemy.strength > sololimit)
            soloenemies.Push(enemy);
    }
    
    int tsleft = ts;
    
    if(ts > sololimit && randfloat(random) > 0.8) {
        //TODO boss
    }
    
    
    int groups =std::max((int)std::round(log(float(ts) / minstr) / log(2)) - 3, 1) + 1;
    
    for(int i=0; i<groups; i++) {
        int gts = tsleft;
        if(i != groups - 1) {
            gts = (tsleft * randfloat(random, 0.75f, 1.5f) / (groups-i));
        }
        
        EnemyType *enemy = nullptr;
        do {
            enemy = &EnemyType::Enemies[randint(random, 0, EnemyType::Enemies.GetSize() - soloenemies.GetSize() - 1)];
        } while(
            enemy->strength != minstr && (
            enemy->strength > gts / 3 || 
            (IsFlyer(enemy->type) && ts < avg * 3) ||
            enemy->strength < gts / 15
        ));
        
        int count = int(std::round(((float)gts/enemy->strength)));
        Enemies.push_back({enemy, count, randfloat(random, 1.0f, 5.0f)});
        tsleft -= enemy->strength*count;
    }
    
}
