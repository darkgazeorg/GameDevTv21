#include "Tower.h"
#include <Gorgon/Widgets/Registry.h>
#include <Gorgon/String/AdvancedTextBuilder.h>

const Size TowerSize = {64, 64};

std::map<std::string, TowerType> TowerType::Towers;

void TowerType::Print(Gorgon::Graphics::Layer &target, Point location, int width, bool highlight) {
    auto &reg = Gorgon::Widgets::Registry::Active();
    auto &printer = reg.Printer();
    
    int height = printer.GetHeight() * 4;
    
    if(highlight)
        target.Draw(location, width, height, reg.Backcolor(Gorgon::Graphics::Color::Hover));
    
    RenderIcon(target, location);
    
    Gorgon::String::AdvancedTextBuilder adv;
    adv.UseHeader(Gorgon::Graphics::HeaderLevel::H3);
    adv.Append(name + "\n");
    adv.UseBoldFont();
    adv.SetTabWidth(0, 100);
    adv.UseBoldFont();
    adv.Append("Cost\t");
    adv.UseDefaultFont();
    adv.Append(cost);
    adv.LineBreak();
    adv.UseBoldFont();
    adv.Append("DPS\t");
    adv.UseDefaultFont();
    adv.Append(round(damageperbullet / reloadtime*10) / 10);
    adv.LineBreak();
    
    printer.AdvancedPrint(target, adv, location + Point(68, 4), width-68, true, false);
}

void TowerType::RenderIcon(Gorgon::Graphics::Layer &target, Point location) {
    if(base)
        base->Draw(target, location + Point(TowerSize - base->GetSize())/2);
    
    if(top.GetCount())
        top[0].Draw(target, location + Point(TowerSize - top[0].GetSize())/2);
    
    if(displaybullets) {
        for(auto loc : bulletlocations) {
            Scale(loc, TowerSize);
            bullet[0].Draw(target, location + loc - Point(bullet[0].GetSize()/2));
        }
    }
}
