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
        towerslayer(Widgets::Registry::Layerbox_Blank),
        enemieslayer(Widgets::Registry::Layerbox_Blank),
        maplayerbox(Widgets::Registry::Layerbox_Blank),
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
        topleftpnl.SetHeight(ui.GetUnitWidth());
        topleftpnl.Move(ui.GetSpacing(), ui.GetSpacing());
        topleftpnl.EnableScroll(false, false);
        topleftpnl.SetWidthInUnits(20);
        
        nextwave.Text = "Next wave";
        nextwave.ClickEvent.Register(*this, &Game::StartNextLevel);
        
        scrapicon = Scale(resources.Root().Get<R::Folder>(2).Get<R::Image>("Scraps"), 
                          Size{Widgets::Registry::Active().GetEmSize()}
        );
        scraplbl.SetIcon(scrapicon);
        scraplbl.Text = Gorgon::String::From(scraps);
        
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
        towerslayer.GetLayer().Add(towersgraphics);
        towerslayer.GetLayer().Add(towersinput);
        
        towersinput.SetClick([this] (Point location){
            if(seltower != -1) {
                if(towers[seltower].UnderConstruction())
                    return;
                
                for(auto l : towerlisting) {
                    if(l.first > location.Y) {
                        if(TowerType::Towers[l.second].GetCost() <= scraps) {
                            auto pos = towers[seltower].GetLocation();
                            
                            towers.erase(towers.begin() + seltower);
                            towers.push_back(Tower(TowerType::Towers[l.second], pos, !levelinprogress));
                            scraps -= TowerType::Towers[l.second].GetCost();
                            seltower = -1;
                            
                            break;
                        }
                    }
                }
            }
            else {
                for(auto l : towerlisting) {
                    if(l.first > location.Y) {
                        if(TowerType::Towers[l.second].GetCost() <= scraps) {
                            if(buildtower == l.second)
                                buildtower = "";
                            else {
                                buildtower = l.second;
                            }
                        }
                        else
                            buildtower = "";
                        
                        break;
                    }
                }
            }
        });
        
        maplayerbox.GetLayer().Add(mapinput);
        maplayerbox.Move(maplayer.GetLocation());
        ui.Add(maplayerbox);
        mapinput.SetMove([this](Point location) {
            Size tilesize = {48, 48};

            maphover = (location - map->offset) / tilesize;
            
            if(maphover.X < 0 || maphover.Y < 0 || maphover.X >= map->GetSize().Width || maphover.Y >= map->GetSize().Height)
                maphover = {-1, -1};
        });
        
        mapinput.SetOut([this]() {
            maphover = {-1, -1};
        });
        
        mapinput.SetDown([this]() {
            if(maphover.X != -1 && buildtower != "" && 
               scraps >= TowerType::Towers[buildtower].GetCost() && 
               (*map)(maphover.X, maphover.Y) == 0
            ) {
                bool found = false;
                for(auto &tower : towers) {
                    if(tower.GetLocation() == maphover) {
                        found = true;
                        break;
                    }
                }
                
                if(found)
                    return;
                
                towers.push_back(Tower(TowerType::Towers[buildtower], maphover, !levelinprogress));
                
                scraps -= TowerType::Towers[buildtower].GetCost();
                
                if(scraps < TowerType::Towers[buildtower].GetCost() || Gorgon::Input::Keyboard::CurrentModifier != Gorgon::Input::Keyboard::Modifier::Shift)
                    buildtower = "";
            }
            else if(maphover.X != -1) {
                int ind = 0;
                for(auto &tower : towers) {
                    if(tower.GetLocation() == maphover) {
                        seltower = ind;
                        return;
                    }
                    
                    ind++;
                }
                
                seltower = -1;
            }
        });
        
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
        scraps = 40;
        delete map;
        map = new Map(random);
        PrepareNextLevel();
        enemyind = 0;
        auto t = TowerType::Towers.begin();
        std::advance(t, 2);
        gamespeed = 1;
        paused = false;
        scrapsinlevel = 0;
        buildtower = "";
    }
    
    void PrepareNextLevel() {
        scraps += scrapsinlevel;
        scrapsinlevel = 0;
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
        maplayerbox.Resize(maplayer.GetEffectiveBounds().GetSize());
        graphics.Clear();
        graphics.Draw(Widgets::Registry::Active().Backcolor(Gorgon::Graphics::Color::Container));
        
        quit.Move(ui.GetWidth() - quit.GetWidth() - ui.GetSpacing(), ui.GetSpacing());
        
        drawtowers();
    }
    
    void drawtowers() {
        towersgraphics.Clear();
        if(seltower == -1) {
            int y = 0;
            towerlisting.clear();
            for(auto &tower : TowerType::Towers) {
                if(tower.second.IsPlacable()) {
                    if(tower.first == buildtower && tower.second.GetCost() > scraps)
                        buildtower = "";
                    towerslayer.SetHeight(y + 68);
                    towerlisting[y + 68] = tower.first;
                    
                    tower.second.Print(towersgraphics, {0, y}, towersgraphics.GetEffectiveBounds().Width(), tower.first == buildtower, tower.second.GetCost() > scraps);
                    y += 68;
                    
                }
            }
        }
        else {
            towerslayer.SetHeight(110);
            auto &cur = towers[seltower];
            cur.Print(towersgraphics, {0, 0}, towersgraphics.GetEffectiveBounds().Width());
            
            if(!cur.UnderConstruction()) {            
                int y = 110;
                towerlisting.clear();
                for(auto s : cur.GetType().GetUpgrades()) {
                    auto &tower = TowerType::Towers[s];
                    
                    towerslayer.SetHeight(y + 68);
                    towerlisting[y + 68] = s;
                    if(s != "") {
                        tower.Print(towersgraphics, {0, y}, towersgraphics.GetEffectiveBounds().Width(), s == buildtower, tower.GetCost() > scraps);
                    }
                }
            }
        }
    }
    
    void drawenemies() {
        enemygraphics.Clear();
        int y = 0;
        for(auto &g : wave.Enemies) {
            enemieslayer.SetHeight(y + 80);
            
            g.enemy->Print(enemygraphics, {0, y}, towersgraphics.GetEffectiveBounds().Width(), g.count);
            y += 80;
        }
    }

    virtual void doframe(unsigned delta) override {
        scraplbl.Text = Gorgon::String::Concat(scraps, " + ", scrapsinlevel);
        healthlbl.Text = Gorgon::String::From(health);
        
        delta *= gamespeed * !paused;

        for(auto &twr : towers)
            scrapsinlevel += twr.Progress(delta, enemies);
        
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
        
        drawtowers();
    }

    virtual void render() override {
        maplayer.Clear();
        maplayer.Draw(Color::Black);
        map->Render(maplayer);
        
        Size tilesize = {48, 48};
        
        gamelayer.Clear();
        for(auto &enemy : enemies)
            enemy.second.Render(gamelayer, map->offset, tilesize);
        
        for(auto &twr : towers)
            twr.Render(gamelayer, map->offset, tilesize);
        
        if(maphover.X != -1 && buildtower != "" && (*map)(maphover.X, maphover.Y) == 0) {
            bool found = false;
            for(auto &tower : towers) {
                if(tower.GetLocation() == maphover) {
                    found = true;
                    break;
                }
            }
            
            if(!found)
                resources.Root().Get<R::Folder>(2).Get<R::Image>("Target").DrawStretched(gamelayer, maphover * tilesize + map->offset, tilesize);
        }
        
        if(seltower != -1) {
            resources.Root().Get<R::Folder>(2).Get<R::Image>("Selected").DrawStretched(gamelayer, towers[seltower].GetLocation() * tilesize - Point(4, 4) + map->offset, tilesize+Size(8, 8));
        }
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
                break;
                
            case Keycode::Escape:
                buildtower = "";
                seltower = -1;
                break;
                
            case Keycode::Enter:
            case Keycode::Numpad_Enter:
                if(maphover.X != -1 && buildtower != "" && scraps >= TowerType::Towers[buildtower].GetCost()) {
                    towers.push_back(Tower(TowerType::Towers[buildtower], maphover, !levelinprogress));
                    
                    scraps -= TowerType::Towers[buildtower].GetCost();
                    buildtower = "";
                }
                break;

            }
        }
    }
    
    std::default_random_engine random = std::default_random_engine{0};
    
    bool inithack = LoadResources();
    
    Gorgon::Graphics::Layer maplayer, towersgraphics, enemygraphics, gamelayer;
    Gorgon::Input::Layer towersinput, mapinput;
    Map *map = nullptr;
    
    int scraps  = 40;
    int scrapsinlevel  = 0;
    int health = 100;
    int curstr = 67; //this will be multiplied with 1.5 to get 100 for the first level
    int level  = 0;
    int delayenemies = 0; //ms
    bool levelinprogress = false;
    int seltower = -1;
    std::string buildtower;
    
    Widgets::Panel topleftpnl;
    Widgets::Button quit, nextwave;
    Widgets::Label scraplbl, healthlbl;
    Widgets::Layerbox towerslayer;
    Widgets::Layerbox enemieslayer;
    Widgets::Layerbox maplayerbox;
    Widgets::Panel towerspnl;
    Widgets::Panel enemiespnl;
    
    Point maphover = {-1, -1};
    
    int gamespeed = 1;
    bool paused   = false;
    
    Gorgon::Graphics::Bitmap scrapicon, healthicon;
    
    Wave wave;
    std::map<long int, Enemy> enemies;
    std::map<int, std::string> towerlisting;
    long int enemyind;
    std::vector<Tower> towers;
};
