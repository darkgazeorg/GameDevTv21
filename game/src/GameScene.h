#pragma once

#include "Scenes.h"
#include "Map.h"
#include <Gorgon/Scene.h>
#include "Types.h"
#include <Gorgon/Widgets/Button.h>
#include <Gorgon/Widgets/Label.h>
#include <Gorgon/Widgets/Layerbox.h>
#include <Gorgon/UI/Organizers/Flow.h>
#include <Gorgon/Graphics/ScalableObject.h>
#include <Gorgon/Resource/Image.h>
#include "ImProc.h"
#include "Resources.h"
#include "Tower.h"
#include "Enemy.h"

extern R::File resources;

class Game : public Gorgon::Scene {
public:
    Game(Gorgon::SceneManager &parent, Gorgon::SceneID id) : 
        Gorgon::Scene(parent, id, true),
        topleftpnl(Widgets::Registry::Panel_Blank),
        towerspnl(Widgets::Registry::Panel_Blank),
        enemiespnl(Widgets::Registry::Panel_Blank)
    { 
        graphics.Add(maplayer);
        maplayer.Move(Widgets::Registry::Active()[Widgets::Registry::Panel_Left].GetHeight(), Widgets::Registry::Active()[Widgets::Registry::Panel_Top].GetHeight());
        maplayer.EnableClipping();
        quit.Text = "Quit";
        quit.ClickEvent.Register([this]() {
            this->parent->Quit();
        });
        ui.Add(quit);
        quit.Move(ui.GetWidth() - quit.GetWidth() - ui.GetSpacing(), ui.GetSpacing());
        
        auto &org = topleftpnl.CreateOrganizer<Gorgon::UI::Organizers::Flow>();
        ui.Add(topleftpnl);
        topleftpnl.SetHeight(Widgets::Registry::Active()[Widgets::Registry::Panel_Left].GetHeight()-ui.GetSpacing());
        topleftpnl.Move(ui.GetSpacing(), ui.GetSpacing());
        topleftpnl.EnableScroll(false, false);
        topleftpnl.SetWidthInUnits(20);
        
        nextwave.Text = "Next wave";
        nextwave.ClickEvent.Register(*this, &Game::StartNextLevel);
        
        scrapicon = Scale(resources.Root().Get<R::Folder>(2).Get<R::Image>("Scraps"), 
                          Size{Widgets::Registry::Active().GetEmSize()}
        );
        scraplbl.SetIcon(scrapicon);
        scraplbl.Text = Gorgon::String::From(scrap);
        
        healthicon = Scale(resources.Root().Get<R::Folder>(2).Get<R::Image>("Health"), 
                          Size{Widgets::Registry::Active().GetEmSize()}
        );
        healthlbl.SetIcon(healthicon);
        healthlbl.Text = Gorgon::String::From(health);
        
        
        
        nextwave.SetHeight(ui.GetUnitWidth());
        org << nextwave << 1 << " " << scraplbl << 1 << " " << healthlbl;
        
        ui.Add(towerspnl);
        towerspnl.SetHeight((ui.GetHeight() - maplayer.GetTop() - ui.GetSpacing())/2);
        towerspnl.SetWidth(maplayer.GetLeft() - ui.GetSpacing());
        towerspnl.Move(0, maplayer.GetTop());
        towerslayer.SetWidth(towerspnl.GetInteriorSize().Width);
        towerspnl.Add(towerslayer);
        towerslayer.GetLayer().Add(towergraphics);
        
        enemiespnl.SetHeight((ui.GetHeight() - maplayer.GetTop() - ui.GetSpacing())/2);
        enemiespnl.SetWidth(maplayer.GetLeft() - ui.GetSpacing());
        enemiespnl.Move(0, towerspnl.GetHeight() + ui.GetSpacing() + maplayer.GetTop());
        enemieslayer.SetWidth(towerspnl.GetInteriorSize().Width);
        ui.Add(enemiespnl);
        enemiespnl.Add(enemieslayer);
        enemieslayer.GetLayer().Add(enemygraphics);
        
        graphics.Add(gamelayer);
        gamelayer.Move(maplayer.GetLocation());
        gamelayer.EnableClipping();
        
        Reset();
    }
    
    ~Game() {
    }
    
    void Reset() {
        scrap = 50;
        delete map;
        map = new Map(random);
        PrepareNextLevel();
    }
    
    void PrepareNextLevel() {
        curstr *= 1.5;
        wave = Wave(curstr, random);
        drawenemies();
        enemy = new Enemy(*wave.Enemies[0].enemy, 0, map->Paths[2]);
    }
    
    void StartNextLevel() {
    }

private:
    virtual void activate() override {
        gamelayer.Resize(maplayer.GetEffectiveBounds().GetSize());
        graphics.Clear();
        graphics.Draw(Widgets::Registry::Active().Backcolor(Gorgon::Graphics::Color::Container));
        
        quit.Move(ui.GetWidth() - quit.GetWidth() - ui.GetSpacing(), ui.GetSpacing());
        
        drawtowers();
    }
    
    void drawtowers() {
        towergraphics.Clear();
        int y = 0;
        for(auto &tower : TowerType::Towers) {
            if(tower.second.IsPlacable()) {
                towerslayer.SetHeight(y + 68);
                
                tower.second.Print(towergraphics, {0, y}, towergraphics.GetEffectiveBounds().Width(), false);
                y += 68;
            }
        }
    }
    
    void drawenemies() {
        enemygraphics.Clear();
        int y = 0;
        for(auto &g : wave.Enemies) {
            enemieslayer.SetHeight(y + 80);
            
            g.enemy->Print(enemygraphics, {0, y}, towergraphics.GetEffectiveBounds().Width(), g.count);
            y += 80;
        }
    }

    virtual void doframe(unsigned delta) override {
        scraplbl.Text = Gorgon::String::From(scrap);
        healthlbl.Text = Gorgon::String::From(health);
        
        if(enemy) {
            auto ret = enemy->Progress(delta);
            
            if(ret > 0) {
                enemy = nullptr;
                health -= ret;
            }
        }
    }

    virtual void render() override {
        maplayer.Clear();
        maplayer.Draw(Color::Black);
        map->Render(maplayer);
        
        gamelayer.Clear();
        if(enemy)
            enemy->Render(gamelayer, map->offset, {48, 48});
    }

    virtual bool RequiresKeyInput() const override {
        return true;
    }

    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
    }
    
    std::default_random_engine random = std::default_random_engine{static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
    
    bool inithack = LoadResources();
    
    Gorgon::Graphics::Layer maplayer, towergraphics, enemygraphics, gamelayer;
    Map *map = nullptr;
    
    int scrap = 30;
    int health = 100;
    int curstr = 67; //this will be multiplied with 1.5 to get 100 for the first level
    
    Widgets::Panel topleftpnl;
    Widgets::Button quit, nextwave;
    Widgets::Label scraplbl, healthlbl;
    Widgets::Layerbox towerslayer;
    Widgets::Layerbox enemieslayer;
    Widgets::Panel towerspnl;
    Widgets::Panel enemiespnl;
    
    Gorgon::Graphics::Bitmap scrapicon, healthicon;
    
    Wave wave;
    Enemy *enemy;
};
