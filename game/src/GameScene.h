#pragma once

#include "Scenes.h"
#include "Map.h"
#include <Gorgon/Scene.h>
#include "Types.h"
#include <Gorgon/Widgets/Button.h>
#include <Gorgon/Resource/Image.h>
#include "ImProc.h"

extern R::File resources;

class Game : public Gorgon::Scene {
public:
    Game(Gorgon::SceneManager &parent, Gorgon::SceneID id) : 
        Gorgon::Scene(parent, id, true) 
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
        
        test = CreateRotations(resources.Root().Get<R::Folder>(2).Get<R::Image>(0).MoveOutAsBitmap(), 32);
        for(auto &bmp : test)
            prov.Add(bmp);
        
    }
    
    ~Game() {
    }

private:
    virtual void activate() override {
        graphics.Clear();
        graphics.Draw(Widgets::Registry::Active().Backcolor(Gorgon::Graphics::Color::Container));
        
        anim = &prov.CreateAnimation();
    }

    virtual void doframe(unsigned delta) override {
    }

    virtual void render() override {
        maplayer.Clear();
        maplayer.Draw(Color::Black);
        map.Render(maplayer);
        //for(int i=0; i<32; i++)
            anim->Draw(maplayer, 0,0);
    }

    virtual bool RequiresKeyInput() const override {
        return true;
    }

    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
    }
    
    std::default_random_engine random = std::default_random_engine{static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
    
    Gorgon::Graphics::Layer maplayer;
    Map map = Map(random);
    Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> test;
    Gorgon::Graphics::BitmapAnimationProvider prov;
    Gorgon::Graphics::BitmapAnimation *anim;
    
    Widgets::Button quit;
};
