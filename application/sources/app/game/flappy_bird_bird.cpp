#include "flappy_bird_bird.h"
#include "view_render.h"
#include "screens_bitmap.h"

#define SCREEN_HEIGHT 64

#define BIRD_START_X 10
#define BIRD_START_Y 25

#define GRAVITY 2
#define JUMP_FORCE 8

void Bird::init()
{
    x = BIRD_START_X;
    y = BIRD_START_Y;
    vy = 0;
}

// update bird position based on velocity and gravity, and cap within screen bounds
void Bird::update()
{
    vy += GRAVITY;
    y += vy;

    if (y < 0)
    {
        y = 0;
    }

    if (y > SCREEN_HEIGHT - BIRD_HEIGHT)
    {
        y = SCREEN_HEIGHT - BIRD_HEIGHT;
    }
}

// make the bird jump by setting an upward velocity
void Bird::flap()
{
    vy = -JUMP_FORCE;
}

void Bird::draw()
{
    view_render.drawBitmap(
        x,
        y,
        bitmap_flappy_bird,
        BIRD_WIDTH,
        BIRD_HEIGHT,
        WHITE);
}