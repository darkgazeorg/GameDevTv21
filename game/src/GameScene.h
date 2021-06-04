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

#include <Gorgon/UI/Dialog.h>

extern R::File resources;

#define rint(min, max)    std::uniform_int_distribution<int>(min, max)(RNG)

class Game : public Gorgon::Scene {
    friend class EndGame;
public:
    Game(Gorgon::SceneManager &parent, Gorgon::SceneID id) : 
        Gorgon::Scene(parent, id, true),
        topleftpnl(Widgets::Registry::Panel_Blank),
        towerslayer(Widgets::Registry::Layerbox_Blank),
        enemieslayer(Widgets::Registry::Layerbox_Blank),
        maplayerbox(Widgets::Registry::Layerbox_Blank),
        towerspnl(Widgets::Registry::Panel_Blank),
        enemiespnl(Widgets::Registry::Panel_Blank)
    {
        inst = this;
        graphics.Add(maplayer);
        maplayer.Move(Widgets::Registry::Active()[Widgets::Registry::Panel_Left].GetHeight(), Widgets::Registry::Active()[Widgets::Registry::Panel_Top].GetHeight());
        maplayer.EnableClipping();
        quit.Text = "Give up";
        quit.ClickEvent.Register([this]() {
            Gorgon::UI::AskYesNo("Exit", "There is no save functionality, are you certain?", [this] {
                End();
            });
        });
        ui.Add(quit);
        quit.Move(ui.GetWidth() - quit.GetWidth() - ui.GetSpacing(), ui.GetSpacing());
        
        auto &org = topleftpnl.CreateOrganizer<Gorgon::UI::Organizers::Flow>();
        ui.Add(topleftpnl);
        topleftpnl.SetHeight(ui.GetUnitWidth());
        topleftpnl.Move(ui.GetSpacing(), ui.GetSpacing());
        topleftpnl.EnableScroll(false, false);
        topleftpnl.SetWidthInUnits(30);
        
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
        
        levellbl.Text = "**Level:** 1";
        
        
        
        nextwave.SetHeight(ui.GetUnitWidth());
        org << nextwave << 1 << " " << 5 << scraplbl << 1 << " " << 3 << healthlbl << 1 << " " << 3 << levellbl;
        
        ui.Add(towerspnl);
        towerspnl.SetHeight((ui.GetHeight() - maplayer.GetTop() - ui.GetSpacing())/2);
        towerspnl.SetWidth(maplayer.GetLeft() - ui.GetSpacing());
        towerspnl.Move(0, maplayer.GetTop());
        towerslayer.SetWidth(towerspnl.GetInteriorSize().Width);
        towerspnl.Add(towerslayer);
        towerslayer.GetLayer().Add(towersgraphics);
        towerslayer.GetLayer().Add(towersinput);
        
        towersinput.SetClick([this] (Point location){
            towerclick(location.Y);
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
                seltower = -1;
                
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
    }
    
    ~Game() {
    }
    
    void Reset(int seed) {
        if(seed == -1)
            seed = Gorgon::PositiveMod(static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()), 32000);
        
        RNG = std::default_random_engine(seed);
        
        scraps = 35;
        delete map;
        map = new Map(RNG);
        towers.clear();
        enemies.clear();
        towerlisting.clear();
        seltower = -1;
        level = 1;
        levelinprogress = false;
        curstr = 67;
        health = 100;
        maphover = {-1, -1};
        enemyind = 0;
        gamespeed = 1;
        paused = false;
        scrapsinlevel = 0;
        buildtower = "";
        totaldamage = 0;
        totalkills  = 0;
        
        this->seed = seed;
        
        PrepareNextLevel();
    }
    
    void towerclick(int location) {
        if(seltower != -1) {
            if(towers[seltower].UnderConstruction())
                return;
            
            for(auto l : towerlisting) {
                if(l.first >= location) {
                    if(TowerType::Towers[l.second].GetCost() <= scraps) {
                        auto pos = towers[seltower].GetLocation();
                        
                        auto damage = towers[seltower].damage;
                        auto kills  = towers[seltower].kills;
                        towers.erase(towers.begin() + seltower);
                        towers.push_back(Tower(TowerType::Towers[l.second], pos, !levelinprogress));
                        scraps -= TowerType::Towers[l.second].GetCost();
                        seltower = towers.size() - 1;
                        towers.back().damage = damage;
                        towers.back().kills  = kills;
                        
                        break;
                    }
                }
            }
        }
        else {
            for(auto l : towerlisting) {
                if(l.first >= location) {
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
    }
    
    void PrepareNextLevel() {
        scraps += scrapsinlevel;
        totalscraps += scrapsinlevel;
        scrapsinlevel = 0;
        curstr *= 1.5;
        wave = Wave(curstr, RNG);
        drawenemies();
        level++;
        levellbl.Text = "**Level:** " + Gorgon::String::From(level);
        nextwave.Enable();
    }
    
    void StartNextLevel() {
        levelinprogress = true;
        delayenemies = 0;
        paused = false;
        nextwave.Disable();
    }
    
    static Game &Get() {
        return *inst;
    }
    
    int GetSeed() const {
        return seed;
    }
    
    void End() {
        for(auto &tower : towers) {
            totaldamage += tower.damage;
            totalkills  += tower.kills;
        }
        
        parent->SwitchScene(EndGameScene);
    }

private:
    virtual void activate() override {
        gamelayer.Resize(maplayer.GetEffectiveBounds().GetSize());
        maplayerbox.Resize(maplayer.GetEffectiveBounds().GetSize());
        graphics.Clear();
        graphics.Draw(Widgets::Registry::Active().Backcolor(Gorgon::Graphics::Color::Container));
        
        quit.Move(ui.GetWidth() - quit.GetWidth() - ui.GetSpacing(), ui.GetSpacing());
        
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
            towerslayer.SetHeight(5*Gorgon::Widgets::Registry::Active().Printer().GetHeight());
            auto &cur = towers[seltower];
            cur.Print(towersgraphics, {0, 0}, towersgraphics.GetEffectiveBounds().Width());
            
            if(!cur.UnderConstruction()) {            
                int y = 5*Gorgon::Widgets::Registry::Active().Printer().GetHeight();
                towerlisting.clear();
                for(auto s : cur.GetType().GetUpgrades()) {
                    auto &tower = TowerType::Towers[s];
                    
                    towerslayer.SetHeight(y + 68);
                    towerlisting[y + 68] = s;
                    if(s != "") {
                        tower.Print(towersgraphics, {0, y}, towersgraphics.GetEffectiveBounds().Width(), s == buildtower, tower.GetCost() > scraps);
                        y += 68;
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
                    health -= ret;
                    enemiestodel.push_back(p.first);
                }
                
                if(health <= 0) {
                    End();
                    return;
                }
            }
            
            for(auto ind : enemiestodel)
                enemies.erase(ind);
            
            if(!wave.Enemies.empty() && delayenemies < delta) {
                auto &grp = wave.Enemies[0];
                int &cnt = grp.count;
                if(IsFlyer(grp.enemy->GetType()) && grp.inrow == 3 && cnt >= 3) {
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[9]});
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[10]});
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[11]});
                    cnt-=3;
                }
                else if(IsFlyer(grp.enemy->GetType()) && grp.inrow == 2 && cnt >= 2) {
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[10]});
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[11]});
                    cnt-=2;
                }
                else if(IsFlyer(grp.enemy->GetType())) {
                    enemies.emplace(enemyind++, Enemy{*grp.enemy, 0, map->Paths[9]});
                    cnt--;
                }
                else if(grp.inrow == 3 && cnt >= 3) {
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
                
                delayenemies = (int)std::round(1000 * grp.enemy->GetSize().Height / grp.enemy->GetSpeed());
                
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
        resources.Root().Get<R::Folder>(2).Get<R::Image>("black").DrawIn(maplayer);
        map->Render(maplayer);
        
        Size tilesize = {48, 48};
        
        gamelayer.Clear();
        
        int ind = 0;
        for(auto &twr : towers) {
            twr.Render(gamelayer, map->offset, tilesize, seltower == ind || buildtower != "");
            ind++;
        }
        
        for(auto &enemy : enemies)
            enemy.second.Render(gamelayer, map->offset, tilesize);
        
        if(maphover.X != -1 && buildtower != "" && (*map)(maphover.X, maphover.Y) == 0) {
            bool found = false;
            for(auto &tower : towers) {
                if(tower.GetLocation() == maphover) {
                    found = true;
                    break;
                }
            }
            
            if(!found) {
                resources.Root().Get<R::Folder>(2).Get<R::Image>("Target").DrawStretched(gamelayer, maphover * tilesize + map->offset, tilesize);
                
                TowerType::Towers[buildtower].DrawRange(gamelayer, maphover * tilesize + map->offset, tilesize);
            }
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
                if(levelinprogress)
                    paused = !paused;
                else
                    StartNextLevel();
                
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
                
            case Keycode::Number_1:
            case Keycode::Numpad_1:
                if(towerlisting.size() > 0) {
                    towerclick(towerlisting.begin()->first);
                }
                break;

            case Keycode::Number_2:
            case Keycode::Numpad_2:
                if(towerlisting.size() > 1) {
                    auto it = towerlisting.begin();
                    std::advance(it, 1);
                    towerclick(it->first);
                }
                break;

            case Keycode::Number_3:
            case Keycode::Numpad_3:
                if(towerlisting.size() > 2) {
                    auto it = towerlisting.begin();
                    std::advance(it, 2);
                    towerclick(it->first);
                }
                break;

            case Keycode::Number_4:
            case Keycode::Numpad_4:
                if(towerlisting.size() > 3) {
                    auto it = towerlisting.begin();
                    std::advance(it, 3);
                    towerclick(it->first);
                }
                break;

            case Keycode::Number_5:
            case Keycode::Numpad_5:
                if(towerlisting.size() > 4) {
                    auto it = towerlisting.begin();
                    std::advance(it, 4);
                    towerclick(it->first);
                }
                break;

            }
        }
    }
    
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
    Widgets::MarkdownLabel levellbl;
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
    int seed;
    double totaldamage;
    int totalkills;
    long long int totalscraps;
    
    static Game *inst;
};
