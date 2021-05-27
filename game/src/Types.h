#pragma once

#include <Gorgon/Geometry/Point.h>
#include <Gorgon/Graphics/Color.h>
#include <Gorgon/Geometry/Size.h>
#include <Gorgon/Geometry/Bounds.h>
#include <Gorgon/Widgets/Registry.h>
#include <random>

using Gorgon::Geometry::Point;
using Gorgon::Geometry::Size;
using Gorgon::Geometry::Pointf;
using Gorgon::Geometry::Sizef;
using Gorgon::Geometry::Bounds;

namespace Widgets = Gorgon::Widgets;
namespace Color = Gorgon::Graphics::Color;

enum class Direction {
    North,
    South,
    East,
    West,
};

enum class DamageType {
    Knetic,
    Explosive,
    Laser
};

enum class EnemyType {
    Infantry = 1,
    Mechanical = 2,
    MechanicalInfantry = Mechanical | Infantry,
    Alien = 8,
    AlienInfantry = Alien | Infantry,
    MechanicalAlien = Mechanical | Alien,
    MechanicalAlienInfantry = Mechanical | Infantry | Alien,
};

enum class TargetType {
    Ground = 1,
    Air = 2,
    GroundAndAir = 3
};
