#include "Tower.h"
#include "Resources.h"
#include <Gorgon/Widgets/Registry.h>
#include <Gorgon/Resource/Image.h>
#include <Gorgon/String/AdvancedTextBuilder.h>

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
                base->bullet[angle].DrawStretched(target, location * tilesize + loc + offset - Point(sz/2), sz);
            }
        }
    }
}

void Tower::Progress(unsigned delta, std::map<long, Enemy>& enemies) {
    if(construction) {
        if(construction > delta)
            construction -= delta;
        else
            construction = 0;
    }
    
    if(tracktarget != -1) {
        //TODO check if we can still track this target
    }
    
    if(tracktarget == -1) {
        
    }
}

