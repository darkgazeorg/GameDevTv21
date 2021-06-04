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

class Credits : public Gorgon::Scene {
public:
    Credits(Gorgon::SceneManager &parent, Gorgon::SceneID id) :
        Scene(parent, id)
    {
        ui.Add(quit);
        quit.Text = "Return to menu";
        quit.Move(50, 110);
        quit.Resize(189, ui.GetUnitWidth()*1.5);
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
        resources.Root().Get<R::Folder>(2).Get<R::Image>("BG").DrawIn(graphics);
        resources.Root().Get<R::Folder>(2).Get<R::Image>("credits").Draw(graphics, 0, 0);
    }
    
    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
        if(key == Gorgon::Input::Keyboard::Keycodes::Escape)
            parent->SwitchScene(MenuScene);
    }
private:
    
    Widgets::Button quit;
};
