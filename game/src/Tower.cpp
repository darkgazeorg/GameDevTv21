#include "Tower.h"
#include "Resources.h"
#include <Gorgon/Widgets/Registry.h>
#include <Gorgon/Resource/Image.h>
#include <Gorgon/String/AdvancedTextBuilder.h>
#include "Enemy.h"

const Size TowerSize = {64, 64};

std::map<std::string, TowerType> TowerType::Towers;

void TowerType::Print(Gorgon::Graphics::Layer &target, Point location, int width, bool highlight, bool disabled) {
    auto &reg = Gorgon::Widgets::Registry::Active();
    auto &printer = reg.Printer();
    
    int height = printer.GetHeight() * 4;
    
    if(highlight)
        target.Draw(location, width, height, reg.Backcolor(Gorgon::Graphics::Color::Hover));
    
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
    adv.Append(round(numberofbullets * damageperbullet / reloadtime*10) / 10);
    adv.LineBreak();
    
    printer.AdvancedPrint(target, adv, location + Point(68, 4), width-68, true, false);
    if(disabled)
        target.SetColor({1.0f});
}

void TowerType::RenderIcon(Gorgon::Graphics::Layer &target, Point location) {
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

void Tower::Render(Gorgon::Graphics::Layer& target, Gorgon::Geometry::Point offset, Gorgon::Geometry::Size tilesize) {
    if(construction) {
        upgrade.DrawStretched(target, location * tilesize + offset, tilesize);
    }
    else {
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
}

void Tower::Progress(unsigned delta, std::map<long, Enemy>& enemies) {
    if(construction) {
        if(construction > delta)
            construction -= delta;
        else
            construction = 0;
        
        return;
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
    
    if(tracktarget == -1) {
        float min = base->range;
        long int ind = -1;
        for(auto &p : enemies) {
            auto dist = p.second.GetLocation().Distance(location) - p.second.GetSize();
            
            if(dist <= min) {
                min = dist;
                ind = p.first;
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
        
        if(currentbullets < base->numberofbullets)
            currentbullets++;
        
        nextfire = 1000 * base->reloadtime / (base->numberofbullets + 1);
    }
    
    if(currentbullets && nextfire < delta && tracktarget != -1) {
        flyingbullets.push_back({tracktarget, base->bulletlocations[currentbullets-1] + location, angle, false, base->bulletlocations[currentbullets-1] + location});
        nextfire = 1000 * base->reloadtime / (base->numberofbullets + 1);
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
    
restart:
    for(auto &bullet : flyingbullets) {
        if(bullet.done)
            continue;
        bullet.done = true;
        
        auto &enemy = enemies.at(bullet.target);
        
        float dist = base->bulletspeed * delta / 1000;
        
        if(dist >= bullet.location.Distance(enemy.GetLocation())) {
            bool removed = false;
            
            if(base->areasize > 0) {
                bullet.location = enemy.GetLocation();
                
                std::vector<long int> eraselist;
                for(auto &p : enemies) {
                    auto dist = p.second.GetLocation().Distance(bullet.location);
                    if(dist < base->areasize) {
                        if(p.second.ApplyDamage(base->damageperbullet * (1 - base->areafalloff * dist / base->areasize), base->damagetype)) {
                            eraselist.push_back(p.first);
                            removed = true;
                        }
                    }
                }
                for(auto ind : eraselist)
                    enemies.erase(ind);
            }
            else {
                auto dist = bullet.start.Distance(enemy.GetLocation());
                
                if(enemy.ApplyDamage(base->damageperbullet * (1 - base->distancefalloff * dist / base->range), base->damagetype)) {
                    enemies.erase(bullet.target);
                    removed = true;
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
}

