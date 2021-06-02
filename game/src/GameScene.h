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

#define rint(min, max)    std::uniform_int_distribution<int>(min, max)(random)

class Game : public Gorgon::Scene {
public:
    Game(Gorgon::SceneManager &parent, Gorgon::SceneID id, int seed) : 
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
        
        if(seed == -1)
            seed = Gorgon::PositiveMod(static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()), 32000);
        std::cout << "Seed: " << seed << std::endl;
        Reset(seed);
    }
    
    ~Game() {
    }
    
    void Reset(int seed) {
        random = std::default_random_engine(seed);
        scrap = 40;
        delete map;
        map = new Map(random);
        PrepareNextLevel();
        enemyind = 0;
        auto t = TowerType::Towers.begin();
        std::advance(t, 2);
        twr = new Tower(t->second, {4, 4}, false);
        gamespeed = 1;
        paused = false;
    }
    
    void PrepareNextLevel() {
        curstr *= 1.5;
        wave = Wave(curstr, random);
        drawenemies();
        level++;
    }
    
    void StartNextLevel() {
        levelinprogress = true;
        delayenemies = 0;
        paused = false;
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
                
                tower.second.Print(towergraphics, {0, y}, towergraphics.GetEffectiveBounds().Width(), false, tower.second.GetCost() > scrap);
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
        
        delta *= gamespeed * !paused;

        twr->Progress(delta, enemies);
        
        if(levelinprogress) {
            std::vector<long int> enemiestodel;
            for(auto &p : enemies) {
                auto &enemy = p.second;
                auto ret = enemy.Progress(delta);
                
                if(ret > 0) {
                    health -= ret/5;
                    enemiestodel.push_back(p.first);
                }
                
                if(health <= 0)
                    parent->Quit(); //for now
            }
            
            for(auto ind : enemiestodel)
                enemies.erase(ind);
            
            if(!wave.Enemies.empty() && delayenemies < delta) {
                auto &grp = wave.Enemies[0];
                int &cnt = grp.count;
                if(grp.inrow == 3 && cnt >= 3) {
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[1]});
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[4]});
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[7]});
                    
                    cnt -= 3;
                }
                else if(grp.inrow == 2 && cnt >= 2) {
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[2]});
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[6]});
                    
                    cnt -= 2;
                }
                else {
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[rint(grp.enemy->GetSize().Width-1, 9-grp.enemy->GetSize().Width)]});
                    cnt--;
                }
                
                delayenemies = (int)std::round(1000 * grp.enemy->GetSize().Height / 1.8f / grp.enemy->GetSpeed());
                
                if(cnt == 0) {
                    delayenemies += (int)std::round(grp.delay * 1000);
                    wave.Enemies.erase(wave.Enemies.begin());
                }
                
                drawenemies();
            }
            else if(delayenemies > delta) {
                delayenemies -= delta;
            }
            
            if(enemies.empty() && wave.Enemies.empty()) {
                levelinprogress = false;
                PrepareNextLevel();
            }
        }
    }

    virtual void render() override {
        maplayer.Clear();
        maplayer.Draw(Color::Black);
        map->Render(maplayer);
        
        gamelayer.Clear();
        for(auto &enemy : enemies)
            enemy.second.Render(gamelayer, map->offset, {48, 48});
        
        twr->Render(gamelayer, map->offset, {48, 48});
    }

    virtual bool RequiresKeyInput() const override {
        return true;
    }

    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
        namespace Keycode = Gorgon::Input::Keyboard::Keycodes;
        
        if(press) {
            switch(key) {
            case Keycode::Number_9:
            case Keycode::Numpad_Minus:
                switch(gamespeed) {
                case 2:
                    gamespeed = 1;
                    break;
                case 3:
                    gamespeed = 2;
                    break;
                case 5:
                    gamespeed = 3;
                    break;
                case 8:
                    gamespeed = 5;
                    break;
                }
                break;
                
            case Keycode::Number_0:
            case Keycode::Numpad_Plus:
                switch(gamespeed) {
                case 1:
                    gamespeed = 2;
                    break;
                case 2:
                    gamespeed = 3;
                    break;
                case 3:
                    gamespeed = 5;
                    break;
                case 5:
                    gamespeed = 8;
                    break;
                }
                break;
                
            case Keycode::Space:
                paused = !paused;
            }
        }
    }
    
    std::default_random_engine random = std::default_random_engine{0};
    
    bool inithack = LoadResources();
    
    Gorgon::Graphics::Layer maplayer, towergraphics, enemygraphics, gamelayer;
    Map *map = nullptr;
    
    int scrap  = 40;
    int health = 100;
    int curstr = 67; //this will be multiplied with 1.5 to get 100 for the first level
    int level  = 0;
    int delayenemies = 0; //ms
    bool levelinprogress = false;
    
    Widgets::Panel topleftpnl;
    Widgets::Button quit, nextwave;
    Widgets::Label scraplbl, healthlbl;
    Widgets::Layerbox towerslayer;
    Widgets::Layerbox enemieslayer;
    Widgets::Panel towerspnl;
    Widgets::Panel enemiespnl;
    
    int gamespeed = 1;
    bool paused   = false;
    
    Gorgon::Graphics::Bitmap scrapicon, healthicon;
    
    Wave wave;
    std::map<long int, Enemy> enemies;
    long int enemyind;
    Tower *twr;
};
