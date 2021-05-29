#include "Resources.h"

#include <Gorgon/Geometry/PointList.h>
#include <Gorgon/Resource/Image.h>
#include <Gorgon/Resource/Data.h>
#include <Gorgon/String/Tokenizer.h>

#include "Tower.h"

R::File resources;

Gorgon::Containers::Collection<Gorgon::Graphics::Bitmap> tilesets;
std::vector<std::vector<Gorgon::Graphics::TextureImage>> tiles;

bool LoadResources() {
    if(resources.Root().GetCount() != 0)
        return true;
    
    resources.LoadFile("Resources_1x.gor");
    resources.Prepare();
    
    auto &tilesfold = resources.Root().Get<R::Folder>(0);
    auto &imagesfold = resources.Root().Get<R::Folder>(1);
    auto &towersfold = resources.Root().Get<R::Folder>(3);
    
    for(auto &res : tilesfold) {
        if(res.GetGID() == R::GID::Image) {
            auto &im = dynamic_cast<R::Image&>(res);
            tilesets.AddNew(im.MoveOutAsBitmap());
        }
    }
    
    for(auto &tileset : tilesets) {
        auto size = tileset.GetHeight()/3;
        
        Gorgon::Geometry::PointList<Point> tilelocations = {
            {1, 1}, {4, 2}, {3, 2}, //empty, full, single
            {0, 1}, {2, 1},         //left, right
            {1, 0}, {1, 2},         //top, bottom,
            //corners are inverted!
            {3, 0}, {4, 0},         //topleft, topright
            {3, 1}, {4, 1},         //bottomleft, bottomright
            //inner
            {0, 0}, {2, 0},         //topleft, topright
            {0, 2}, {2, 2},         //bottomleft, bottomright
        };
        
        Scale(tilelocations, size);
        
        std::vector<Gorgon::Geometry::Bounds> tilebounds;
        for(auto p : tilelocations) {
            tilebounds.push_back({p, size, size});
        }
        tiles.push_back(tileset.CreateAtlasImages(tilebounds));
    }
    
    for(auto &res : towersfold) {
        if(res.GetGID() == R::GID::Data) {
            auto &data = dynamic_cast<R::Data&>(res);
            
            auto ret = TowerType::Towers.insert(std::make_pair(data.GetName(), TowerType{}));
            
            TowerType &tower = ret.first->second;
            
            tower.id                = ret.first->first;
            tower.name              = data.Get<std::string>(0);
            tower.damageperbullet   = data.Get<int>(1);
            tower.reloadtime        = data.Get<float>(2);
            tower.numberofbullets   = data.Get<int>(3);
            tower.continuousreload  = data.Get<int>(4);
            tower.bulletspeed       = data.Get<float>(5);
            tower.bulletacceleration= data.Get<float>(17);
            tower.damagetype        = (DamageType)data.Get<int>(6);
            tower.areasize          = data.Get<float>(7);
            tower.distancefalloff   = data.Get<float>(8);
            tower.areafalloff       = data.Get<float>(9);
            tower.cost              = data.Get<int>(10);
            tower.range             = data.Get<float>(12);
            tower.placable          = data.Get<int>(13);
            tower.target            = (TargetType)data.Get<int>(14);
            tower.effectiveagainst  = (EnemyType)data.Get<int>(15);
            tower.effectivemultiplier= data.Get<float>(16);
            auto str = data.Get<std::string>(19);
            if(!str.empty())
                tower.base          = &imagesfold.Get<R::AnimationStorage>(str);
            str = data.Get<std::string>(20);
            if(!str.empty())
                tower.top           = &imagesfold.Get<R::AnimationStorage>(str);
            str = data.Get<std::string>(21);
            if(!str.empty())
                tower.effect        = &imagesfold.Get<R::AnimationStorage>(str);
            str = data.Get<std::string>(22);
            if(!str.empty())
                tower.bullet        = &imagesfold.Get<R::AnimationStorage>(str);
            str = data.Get<std::string>(23);
            if(!str.empty())
                tower.bulleteffect  = &imagesfold.Get<R::AnimationStorage>(str);
            
            str = data.Get<std::string>(18);
            std::stringstream ss(str);
            Pointf pnt;
            while(ss >> pnt) {
                tower.bulletlocations.push_back(pnt);
            }
            
            Gorgon::String::Tokenizer tok(data.Get<std::string>(11), ",");
            for(; tok.IsValid(); tok.Next()) {
                tower.upgradesto.push_back(*tok);
            }
        }
    }
    
    return true;
}
