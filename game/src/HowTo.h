#pragma once

#include "Scenes.h"
#include <Gorgon/Scene.h>
#include "Types.h"
#include <Gorgon/Widgets/Button.h>
#include <Gorgon/Widgets/Label.h>
#include <Gorgon/Widgets/Layerbox.h>
#include <Gorgon/UI/Dialog.h>
#include "Resources.h"
#include "GameScene.h"

#define rint(min, max)    std::uniform_int_distribution<int>(min, max)(RNG)

class HowTo : public Gorgon::Scene {
public:
    HowTo(Gorgon::SceneManager &parent, Gorgon::SceneID id) :
        Scene(parent, id)
    {
        ui.Add(quit);
        quit.Text = "Return to menu";
        quit.Move(68, 120);
        quit.Resize(300, ui.GetUnitWidth()*1.5);
        quit.ClickEvent.Register([&] {
            parent.SwitchScene(MenuScene);
        });
    }
    
    virtual bool RequiresKeyInput() const override {
        return true;
    }
    
    virtual void render() override {
    }
    
    void doframe(unsigned int delta) override {
    }
    
    virtual void activate() override {
        graphics.Clear();
        resources.Root().Get<R::Folder>(2).Get<R::Image>("BG2").DrawIn(graphics);
        resources.Root().Get<R::Folder>(2).Get<R::Image>("howto").Draw(graphics, 0, 0);
    }
    
    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
        if(key == Gorgon::Input::Keyboard::Keycodes::Escape)
            parent->SwitchScene(MenuScene);
    }
private:
    
    Widgets::Button quit;
};
