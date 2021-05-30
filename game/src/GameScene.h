#pragma once

#include "Scenes.h"
#include "Map.h"
#include <Gorgon/Scene.h>
#include "Types.h"
#include <Gorgon/Widgets/Button.h>
#include <Gorgon/Widgets/Label.h>
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
        topleftpnl(Widgets::Registry::Panel_Blank)
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
        nextwave.Disable();
        
        scrapicon = Scale(resources.Root().Get<R::Folder>(2).Get<R::Image>("Scraps"), 
                          Size{Widgets::Registry::Active().GetEmSize()}
        );
        scraplbl.SetIcon(scrapicon);
        scraplbl.Text = Gorgon::String::From(scrap);
        
        nextwave.SetHeight(ui.GetUnitWidth());
        org << nextwave << 1 << " " << scraplbl;
    }
    
    ~Game() {
    }
    
    void Reset() {
        scrap = 50;
        delete map;
        map = new Map(random);
    }

private:
    virtual void activate() override {
        graphics.Clear();
        graphics.Draw(Widgets::Registry::Active().Backcolor(Gorgon::Graphics::Color::Container));
        int y = maplayer.GetTop();
        for(auto &tower : TowerType::Towers) {
            if(tower.second.IsPlacable()) {
                tower.second.Print(graphics, {0, y}, maplayer.GetLeft() - 4, false);
                y += 68;
            }
        }
        quit.Move(ui.GetWidth() - quit.GetWidth() - ui.GetSpacing(), ui.GetSpacing());
        
        Wave w(800, random);
        for(auto &g : w.Enemies) {
            std::cout << g.enemy->name << " x" << g.count << std::endl;
        }
    }

    virtual void doframe(unsigned delta) override {
        scraplbl.Text = Gorgon::String::From(scrap);
    }

    virtual void render() override {
        maplayer.Clear();
        maplayer.Draw(Color::Black);
        map->Render(maplayer);
    }

    virtual bool RequiresKeyInput() const override {
        return true;
    }

    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
    }
    
    std::default_random_engine random = std::default_random_engine{static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
    
    bool inithack = LoadResources();
    
    Gorgon::Graphics::Layer maplayer;
    Map *map = new Map(random);
    
    int scrap = 50;
    
    Widgets::Panel topleftpnl;
    Widgets::Button quit, nextwave;
    Widgets::Label scraplbl;
    
    Gorgon::Graphics::Bitmap scrapicon;
};
