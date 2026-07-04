#pragma once

enum TEXTURE_ID
{
    TEX_MARIO = 0,
    TEX_COMMON1 = 1,
    TEX_COMMON2 = 2,
    TEX_FIRE_MARIO = 3,
    TEX_SCISSORS_MARIO = 4,
    TEX_ENEMIES_1 = 10, // enemies_transparent.png (Goomba, Green Koopa walk)
    TEX_ENEMIES_2 = 15, // enemies_transparent_3.png (Koopa shell, Red Koopa, Flying Koopa)
    TEX_HUD = 20,
    TEX_INTRO = 30,
    TEX_BBOX = 99,
    TEX_ENEMY_TEST = 100,

    TEX_LEVEL_CLEAR = 701,
    TEX_GAME_OVER = 702,
    TEX_YOU_WIN = 703,
    TEX_MAP_LEVEL = 800,
    TEX_OBTAIN_ITEM = 900,
    TEX_WHITE = 999,

    TEX_GRASS_BRICK = 1003,
    TEX_CLOUDS = 1005
};

class ResourceLoader
{
  public:
    static void LoadAll();
};
