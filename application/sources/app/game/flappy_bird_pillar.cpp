#include "flappy_bird_pillar.h"
#include "view_render.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PILLAR_TYPE_COUNT 3

// initialize pillar with default position and gap
void Pillar::init()
{
    x = 90;

    gap = 42;
    gapTop = 6;

    type = 0;

    baseSpeed = 1;
    speed = baseSpeed;
}

// set pillar speed based on user setting, and apply it to current speed as well
void Pillar::applyDifficulty(int score)
{
    speed = baseSpeed + (score / 2);

    if (speed < AR_GAME_SETTING_METEOROID_SPEED_MIN)
        speed = AR_GAME_SETTING_METEOROID_SPEED_MIN;

    if (speed > AR_GAME_SETTING_METEOROID_SPEED_MAX)
        speed = AR_GAME_SETTING_METEOROID_SPEED_MAX;
}

// move pillar to the left, and reset when it goes off screen
void Pillar::update()
{
    x -= speed;

    if (x < -PILLAR_WIDTH)
    {
        nextVariant();
    }
}

// draw pillar as two rectangles with a gap in between
void Pillar::nextVariant()
{
    static const int gapSize[PILLAR_TYPE_COUNT] = {42, 48, 38};
    static const int gapTopList[PILLAR_TYPE_COUNT] = {6, 14, 22};
    static const int spawnOffset[PILLAR_TYPE_COUNT] = {0, 10, 18};

    type = (type + 1) % PILLAR_TYPE_COUNT;

    gap = gapSize[type];
    gapTop = gapTopList[type];

    x = SCREEN_WIDTH + spawnOffset[type];
}

// draw pillar as two rectangles with a gap in between
void Pillar::draw()
{
    int bottomY = gapTop + gap;

    view_render.fillRect(x, 0, PILLAR_WIDTH, gapTop, WHITE);

    view_render.fillRect(x - 3, gapTop - 3, PILLAR_WIDTH + 6, 3, WHITE);

    view_render.fillRect(x, bottomY, PILLAR_WIDTH, SCREEN_HEIGHT - bottomY, WHITE);

    view_render.fillRect(x - 3, bottomY, PILLAR_WIDTH + 6, 3, WHITE);
}