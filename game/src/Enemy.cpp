#include "Enemy.h"
#include <Gorgon/Widgets/Registry.h>
#include <Gorgon/String/AdvancedTextBuilder.h>

Gorgon::Containers::Collection<EnemyType> EnemyType::Enemies;


const Size EnemySize = {64, 64};

void EnemyType::Print(Gorgon::Graphics::Layer& target, Gorgon::Geometry::Point location, int width, int count) {
    auto &reg = Gorgon::Widgets::Registry::Active();
    auto &printer = reg.Printer();
    
    RenderIcon(target, location);
    
    Gorgon::String::AdvancedTextBuilder adv;
    adv.UseHeader(Gorgon::Graphics::HeaderLevel::H3);
    adv.Append(name);
    if(count) {
        adv.UseBoldFont();
        adv.Append(" x" + Gorgon::String::From(count));
    }
    adv.Append("\n");
    adv.UseBoldFont();
    adv.SetTabWidth(0, 170);
    adv.Append("HP \t\t");
    adv.UseDefaultFont();
    adv.Append(hitpoints);
    adv.UseBoldFont();
    adv.LineBreak();
    adv.Append("Armor\t");
    adv.UseDefaultFont();
    adv.Append(armor);
    adv.Append("/");
    adv.Append(reactivearmor);
    adv.Append("/");
    adv.Append(shield);
    
    printer.AdvancedPrint(target, adv, location + Point(68, 4), width-68, true, false);
}


void EnemyType::RenderIcon(Gorgon::Graphics::Layer &target, Point location) {
    if(!image.GetCount())
        return;
    
    auto imsize = image[0].GetSize();
    
    if(imsize.Width > EnemySize.Width || imsize.Height > EnemySize.Height) {
        float factor;
        if((float)imsize.Width/EnemySize.Width > (float)imsize.Height/EnemySize.Height)
            factor = (float)EnemySize.Width / imsize.Width;
        else 
            factor = (float)EnemySize.Height / imsize.Height;
        
        image[0].DrawStretched(target, location + Point(EnemySize - imsize*factor)/2, imsize * factor);
    }
    else {
        image[0].Draw(target, location + Point(EnemySize - imsize)/2);
    }
}

float randfloat(std::default_random_engine &random, float min = 0, float max = 1) {
    return std::uniform_real_distribution<float>(min, max)(random);
}

int randint(std::default_random_engine &random, int min, int max) {
    return std::uniform_int_distribution<int>(min, max)(random);
}

bool Enemy::ApplyDamage(int damage, DamageType type) {
    float evaderoll = std::uniform_real_distribution<float>()(RNG);
    if(evaderoll < base->evasion)
        return false;
    
    switch(type) {
    case DamageType::Knetic:
        damage -= base->armor;
        break;
    case DamageType::Explosive:
        damage -= base->reactivearmor;
        break;
    case DamageType::Laser:
        damage -= base->shield;
        break;
    }
    
    if(damage <= 0)
        damage = 1;

    if(hpleft <= damage)
        return true;

    hpleft -= damage;
    return false;
}


void Enemy::Render(Gorgon::Graphics::Layer& target, Point offset, Gorgon::Geometry::Size tilesize) {
    auto &path = *this->path;
    auto st = path[locationpoint];
    auto ed = path.GetLine(locationpoint).End;
    int ind = (int)std::round(atan2(ed.Y - st.Y, ed.X - st.X)/Gorgon::PI*-16+32) % 32;
    Pointf pnt = (st * (1-offsetfrompoint) + ed * offsetfrompoint);
    auto sz = base->image[ind].GetSize() * tilesize / EnemySize;
    pnt = pnt * tilesize;
    pnt -= Point(sz / 2);
    
    if(base->shadow.GetCount()) {
        base->shadow[ind].DrawStretched(target, Point(pnt)+offset, sz);
    }
    
    if(IsFlyer(base->type)) {
        base->image[ind].DrawStretched(target, Point(pnt)+offset-Point(tilesize.Width/2, tilesize.Height), sz);
    }
    else {
        base->image[ind].DrawStretched(target, Point(pnt)+offset, sz);
    }
    
    if(hpleft < base->hitpoints) {
        Point additionaloff = {0, 0};
        if(IsFlyer(base->type))
            additionaloff = {-tilesize.Width/2, -tilesize.Height};
            
        target.Draw(Point(pnt) + offset + Point(0, -6)+additionaloff, sz.Width, 6, Color::Charcoal);
        target.Draw(Point(pnt) + offset + Point(1, 1-6)+additionaloff, (sz.Width-2) * hpleft / base->hitpoints, 4, Color::Red);
    }
}

int Enemy::Progress(int delta) {
    auto movement = base->speed * delta / 1000;
    auto &path = *this->path;
    
    while(movement > 0) {
        if(locationpoint == path.GetCount()-1) {
            //report point loss
            return std::min(std::max(1, base->strength/10), 10) + int(log(base->strength));
        }
        
        auto total = path.GetLine(locationpoint).End.ManhattanDistance(path[locationpoint]);
        auto distleft = total * (1- offsetfrompoint);
        
        if(distleft < movement) {
            movement -= total;
            offsetfrompoint = 0;
            locationpoint++;
        }
        else {
            distleft -= movement;
            movement = 0;
            offsetfrompoint = 1 - distleft / total;
        }
    }
    
    return 0;
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
        int row = 1;
        float mult = 1.0f;
        float delay = randfloat(random, 1.0f, 5.0f);
        if(i != groups - 1) {
            gts = (tsleft * randfloat(random, 0.75f, 1.5f) / (groups-i));
        }
        
        EnemyType *enemy = nullptr;
        do {
            enemy = &EnemyType::Enemies[randint(random, 0, EnemyType::Enemies.GetSize() - soloenemies.GetSize() - 1)];
        } while(
            (enemy->strength != minstr && (
                enemy->strength > gts / 3 || 
                (IsFlyer(enemy->type) && ts < avg * 3))) ||
            (enemy->strength != maxstr && enemy->strength < gts / 15)
        );
        
        mult += (delay - 2.f) / 20.f;
        
        if(!IsFlyer(enemy->type) && enemy->GetSize().Width <= 0.51) {
            row = randint(random, 1, 3);
            if(row == 3)
                mult *= 1.2;
        }
        else if(!IsFlyer(enemy->type) && enemy->GetSize().Width <= 1.01) {
            row = randint(random, 1, 2);
        }
        
        int count = int(std::round(((float)gts/(enemy->strength*mult))));
        Enemies.push_back({enemy, count, row, delay});
        tsleft -= enemy->strength*count;
    }
    
}
