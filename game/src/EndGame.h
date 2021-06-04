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
#include <Gorgon/String/AdvancedTextBuilder.h>

#define rint(min, max)    std::uniform_int_distribution<int>(min, max)(RNG)

class EndGame : public Gorgon::Scene {
public:
    EndGame(Gorgon::SceneManager &parent, Gorgon::SceneID id) :
        Scene(parent, id)
    {
        ui.Add(quit);
        quit.Text = "Return to menu";
        quit.Move(83, 110);
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
        resources.Root().Get<R::Folder>(2).Get<R::Image>("black").DrawIn(graphics);
        resources.Root().Get<R::Folder>(2).Get<R::Image>("gameover").Draw(graphics, 0, 0);
        
        auto &game = Game::Get();
        
        auto y = ui.GetUnitWidth()*2 + 110;
        
        auto &reg = Gorgon::Widgets::Registry::Active();
        auto &printer = reg.Printer();
        Gorgon::String::AdvancedTextBuilder adv;
        adv.SetColor(Color::White)
           .SetTabWidth(240, 0)
           .UseHeader(Gorgon::Graphics::HeaderLevel::H2)
           .Append("Reached level\t")
           .SetFont(Gorgon::Graphics::NamedFont::Larger)
           .Append(game.level)
           .LineBreak()
           .UseHeader(Gorgon::Graphics::HeaderLevel::H2)
           .Append("Total kills\t")
           .SetFont(Gorgon::Graphics::NamedFont::Larger)
           .Append(game.totalkills)
           .LineBreak()
           .UseHeader(Gorgon::Graphics::HeaderLevel::H2)
           .Append("Total damage\t")
           .SetFont(Gorgon::Graphics::NamedFont::Larger)
           .Append(std::round(game.totaldamage))
           .LineBreak()
           .UseHeader(Gorgon::Graphics::HeaderLevel::H2)
           .Append("Total scraps\t")
           .SetFont(Gorgon::Graphics::NamedFont::Larger)
           .Append(game.totalscraps)
           .LineBreak()
           .UseHeader(Gorgon::Graphics::HeaderLevel::H2)
           .Append("Seed\t")
           .SetFont(Gorgon::Graphics::NamedFont::Larger)
           .Append(game.seed)
           .LineBreak()
        ;
        
        printer.Print(graphics, adv, 83, y);
    }
    
    virtual void KeyEvent(Gorgon::Input::Key key, float press) override {
        if(key == Gorgon::Input::Keyboard::Keycodes::Escape)
            parent->SwitchScene(MenuScene);
    }
private:
    
    Widgets::Button quit;
};
