#pragma once

#include <Gorgon/Geometry/Point.h>
#include <Gorgon/Graphics/Color.h>
#include <Gorgon/Geometry/Size.h>
#include <Gorgon/Geometry/Bounds.h>
#include <Gorgon/Widgets/Registry.h>
#include <random>
#include <Gorgon/Resource/File.h>

using Gorgon::Geometry::Point;
using Gorgon::Geometry::Size;
using Gorgon::Geometry::Pointf;
using Gorgon::Geometry::Sizef;
using Gorgon::Geometry::Bounds;

namespace Widgets = Gorgon::Widgets;
namespace Color = Gorgon::Graphics::Color;

enum class Direction {
    East = 0,
    West = 1,
    North = 2,
    South = 3,
    End = 4,
};

enum class DamageType {
    Knetic,
    Explosive,
    Laser
};

enum class EnemyClass {
    Infantry = 1,
    Mechanical = 2,
    Flyer = 4,
    Alien = 8,
};

inline bool IsFlyer(EnemyClass type) {
    return int(type) & int(EnemyClass::Flyer);
}

enum class TargetType {
    Ground = 1,
    Air = 2,
    GroundAndAir = 3
};

namespace R = Gorgon::Resource;
