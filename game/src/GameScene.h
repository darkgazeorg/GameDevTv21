#pragma once

#include "Scenes.h"
#include "Map.h"
#include <Gorgon/Scene.h>


class Game : public Gorgon::Scene {
public:
    Game(Gorgon::SceneManager &parent, Gorgon::SceneID id) : 
        Gorgon::Scene(parent, id, true) 
    { }
    
    ~Game() {
    }

private:
    virtual void activate() override {
    }

    virtual void doframe(unsigned delta) override {
    }

    virtual void render() override {
        graphics.Clear();
        map.Render(graphics);
    }

    virtual bool RequiresKeyInput() const override {
        return true;
    }

    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
    }
    
    Map map;
};
