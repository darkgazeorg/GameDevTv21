#include "Tower.h"
#include "Resources.h"
#include <Gorgon/Widgets/Registry.h>
#include <Gorgon/Resource/Image.h>
#include <Gorgon/String/AdvancedTextBuilder.h>
#include "Enemy.h"

const Size TowerSize = {64, 64};

std::map<std::string, TowerType> TowerType::Towers;

void TowerType::Print(Gorgon::Graphics::Layer &target, Point location, int width, bool highlight, bool disabled) const {
    auto &reg = Gorgon::Widgets::Registry::Active();
    auto &printer = reg.Printer();
    
    if(highlight)
        target.Draw(location, width, 64, reg.Backcolor(Gorgon::Graphics::Color::Hover));
    
    if(disabled)
        target.SetColor({1.0f, 0.5f});
    
    RenderIcon(target, location);
    
    Gorgon::String::AdvancedTextBuilder adv;
    adv.UseHeader(Gorgon::Graphics::HeaderLevel::H3);
    adv.Append(name + "\n");
    adv.UseBoldFont();
    adv.SetTabWidth(0, 100);
    adv.UseBoldFont();
    if(disabled)
        adv.SetColor(Color::Error);
    adv.Append("Cost\t");
    adv.UseDefaultFont();
    adv.Append(cost);
    adv.LineBreak();
    adv.UseBoldFont();
    if(disabled)
        adv.UseDefaultColor();
    adv.Append("DPS\t");
    adv.UseDefaultFont();
    adv.Append(DPS());
    adv.LineBreak();
    
    printer.AdvancedPrint(target, adv, location + Point(68, 4), width-68, true, false);
    if(disabled)
        target.SetColor({1.0f});
}

void TowerType::RenderIcon(Gorgon::Graphics::Layer &target, Point location) const {
    if(base)
        base->Draw(target, location + Point(TowerSize - base->GetSize())/2);
    
    if(top.GetCount())
        top[0].Draw(target, location + Point(TowerSize - top[0].GetSize())/2);
    
    if(displaybullets && bullet.GetCount()) {
        for(auto loc : bulletlocations) {
            Scale(loc, TowerSize);
            bullet[0].Draw(target, location + loc - Point(bullet[0].GetSize()/2));
        }
    }
}

void TowerType::DrawRange(Gorgon::Graphics::Layer& target, Gorgon::Geometry::Point offset, Gorgon::Geometry::Size tilesize) {
    auto sz = rangeimage.GetSize() * tilesize / TowerSize;
    rangeimage.DrawStretched(target, offset - Point(sz/2)+Point(tilesize)/2, sz);
}

void Tower::Render(Gorgon::Graphics::Layer& target, Gorgon::Geometry::Point offset, Gorgon::Geometry::Size tilesize, bool showrange) {
    if(construction) {
        upgrade.DrawStretched(target, location * tilesize + offset, tilesize);
    }
    else {
        if(showrange) {
            auto sz = base->rangeimage.GetSize() * tilesize / TowerSize;
            base->rangeimage.DrawStretched(target, Point(location * tilesize) + offset - Point(sz/2)+Point(tilesize)/2, sz);
        }
        if(base->base) {
            auto sz = base->base->GetSize() * tilesize / TowerSize;
            base->base->DrawStretched(target, location * tilesize + offset, sz);
        }
        
        if(base->top.GetCount()) {
            auto sz = base->top[angle].GetSize() * tilesize / TowerSize;
            base->top[angle].DrawStretched(target, location * tilesize + offset, sz);
        }
        
        if(base->displaybullets && base->bullet.GetCount()) {
            int ind = 0;
            for(auto loc : base->bulletlocations) {
                if(ind == currentbullets)
                    break;
                
                Scale(loc, tilesize);
                auto sz = base->bullet[angle].GetSize() * tilesize / TowerSize;
                base->bullet[angle].DrawStretched(target, Point(location * tilesize) + loc + offset - Point(sz/2), sz);
                ind++;
            }
        }
    }
    
    for(auto &bullet : flyingbullets) {
        auto sz = base->bullet[bullet.angle].GetSize() * tilesize / TowerSize;
        base->bullet[bullet.angle].DrawStretched(target, Point(bullet.location * tilesize) + offset - Point(sz/2), sz);
    }
    
    for(auto &exp : explosions) {
        auto sz = base->bulleteffect->GetSize() * tilesize / TowerSize;
        base->bulleteffect->DrawStretched(target, Point(exp.first * tilesize) + offset - Point(sz/2), sz, Gorgon::Graphics::RGBAf{1.f, (float)pow(1.f - abs(exp.second), 2.f)});
    }
}


int Tower::Progress(unsigned delta, std::map<long, Enemy>& enemies) {
    int scraps = 0;
    if(construction) {
        if(construction > delta)
            construction -= delta;
        else
            construction = 0;
        
        return 0;
    }
    
    if(tracktarget != -1) {
        if(!enemies.count(tracktarget)) {
            tracktarget = -1;
        }
        else {
            auto &t = enemies.at(tracktarget);
            if(t.GetLocation().Distance(location) - t.GetSize() > base->range) {
                tracktarget = -1;
            }
        }
    }
    
    auto cantarget = [this](auto &enemy) {
        bool cantarget = false;
        if(IsFlyer(enemy.GetType())) {
            if((int)base->target&(int)TargetType::Air)
                cantarget = true;
        }
        else {
            if((int)base->target&(int)TargetType::Ground)
                cantarget = true;
        }
        
        return cantarget;
    };
    
    if(tracktarget == -1) {
        float min = base->range;
        long int ind = -1;
        for(auto &p : enemies) {
            auto dist = p.second.GetLocation().Distance(location) - p.second.GetSize();
            
            if(dist <= min) {                
                if(cantarget(p.second)) {
                    min = dist;
                    ind = p.first;
                }
            }
        }
        
        tracktarget = ind;
    }
    
    if(tracktarget != -1) {
        auto &t = enemies.at(tracktarget);
        auto dif = t.GetLocation() - location;
        
        angle = (int)std::round(atan2(dif.Y, dif.X)/Gorgon::PI*-16+24) % 32;
        
    }
    
    reloadloop += delta;
    
    if(reloadloop > base->reloadtime*1000) {
        reloadloop -= base->reloadtime*1000;
        
        if(currentbullets < base->numberofbullets) {
            if(base->continuousreload)
                currentbullets++;
            else
                currentbullets = base->numberofbullets;
        }
        
        nextfire = 1000 * base->reloadtime / (base->numberofbullets * (1+(base->continuousreload==0)) + 1);
    }
    
    if(currentbullets && nextfire < delta && tracktarget != -1) {
        flyingbullets.push_back({tracktarget, base->bulletlocations[(currentbullets-1)%base->bulletlocations.size()] + location, angle, false, base->bulletlocations[currentbullets-1] + location});
        nextfire = 1000 * base->reloadtime / (base->numberofbullets * (1+(base->continuousreload==0)) + 1);
        currentbullets--;
    }
    else if(nextfire > 0) {
        if(nextfire <= delta)
            nextfire = 0;
        else
            nextfire -= delta;
    }
    
    for(auto &bullet : flyingbullets) {
        bullet.done = false;
    }

    auto erasegone = [&]() {
        //remove if the target is gone
        flyingbullets.erase(
            std::remove_if(flyingbullets.begin(), flyingbullets.end(), [&](auto &bullet) {
                return enemies.count(bullet.target) == 0;
            }), flyingbullets.end()
        );
    };
    
    erasegone();
    
    for(auto &exp : explosions) {
        if(exp.second >= 0) {
            exp.second += delta/250.f;
        }
        else {
            exp.second += delta/100.f;
            if(exp.second > 0)
                exp.second = 0;
        }
    }
    
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](auto &exp) {
        return exp.second >= 1.0f;
    }), explosions.end());
    
restart:
    for(auto &bullet : flyingbullets) {
        if(bullet.done)
            continue;
        bullet.done = true;
        
        auto &enemy = enemies.at(bullet.target);
        
        float dist = base->bulletspeed * delta / 1000;
        
        if(dist >= bullet.location.Distance(enemy.GetLocation())) {
            bool removed = false;
            bullet.location = enemy.GetLocation();
            
            if(base->bulleteffect) {
                explosions.push_back({bullet.location, -0.75f});
            }
            
            if(base->areasize > 0) {
                
                std::vector<long int> eraselist;
                for(auto &p : enemies) {
                    auto dist = p.second.GetLocation().Distance(bullet.location);
                    if(dist < base->areasize && cantarget(p.second)) {
                        auto damage = (int)std::round(base->damageperbullet * (1 - base->areafalloff * dist / base->areasize));
                        this->damage += damage;
                        if(p.second.ApplyDamage(damage, base->damagetype)) {
                            eraselist.push_back(p.first);
                            scraps += p.second.GetScraps();
                            removed = true;
                            kills++;
                        }
                    }
                }
                for(auto ind : eraselist)
                    enemies.erase(ind);
            }
            else {
                auto dist = bullet.start.Distance(enemy.GetLocation());
                auto damage = (int)std::round(base->damageperbullet * (1 - base->distancefalloff * dist / base->range));
                this->damage += damage;
                if(enemy.ApplyDamage(damage, base->damagetype)) {
                    scraps += enemy.GetScraps();
                    enemies.erase(bullet.target);
                    removed = true;
                    kills++;
                }
            }
            
            bullet.target = -1; //no target
            if(removed) {
                erasegone();
                goto restart;
            }
        }
        else {
            auto norm = (enemy.GetLocation() - bullet.location).Normalize();
            bullet.angle =  (int)std::round(atan2(norm.Y, norm.X)/Gorgon::PI*-16+24) % 32;
            
            bullet.location += norm * dist;
        }
    }
    return scraps;
}

void Tower::Print(Gorgon::Graphics::Layer& target, Gorgon::Geometry::Point location, int width) {
    auto &reg = Gorgon::Widgets::Registry::Active();
    auto &printer = reg.Printer();
    
    Gorgon::String::AdvancedTextBuilder adv;
    adv.UseHeader(Gorgon::Graphics::HeaderLevel::H3);
    adv.Append(base->name + "\n");
    adv.UseBoldFont();
    adv.SetTabWidth(0, 100);
    adv.UseBoldFont();
    adv.LineBreak();
    if(UnderConstruction()) {
        adv.SetColor(Color::Error)
           .Append("Under construction: ")
           .UseDefaultColor()
           .Append(construction/1000)
           .Append("s")
        ;
    }
    else {
        adv.UseBoldFont()
           .Append("DPS\t\t")
           .UseDefaultFont()
           .Append(base->DPS())
           .LineBreak()
           .UseBoldFont()
           .Append("Kills\t\t")
           .UseDefaultFont()
           .Append(kills)
           .UseBoldFont()
           .Append("\tDamage\t")
           .UseDefaultFont()
           .Append(damage)
           .LineBreak()
        ;
        
        if(base->upgradesto.size()) {
            adv.UseHeader(Gorgon::Graphics::HeaderLevel::H3);
            adv.Append("\nUpgrades: ");
        }
    }
    
    printer.AdvancedPrint(target, adv, location + Point(4, 4), width, true, false);
}
