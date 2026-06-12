#include "flappy_bird_background.h"
#include "view_render.h"

#define SCREEN_WIDTH 128

void Background::init()
{
    cloudX1 = 20;
    cloudX2 = 60;
    cloudX3 = 100;

    skyPhase = 0;
}
void Background::update(int score)
{
    // clouds move slowly
    cloudX1 -= 1;
    cloudX2 -= 1;
    cloudX3 -= 1;

    if (cloudX1 < -10) cloudX1 = SCREEN_WIDTH;
    if (cloudX2 < -10) cloudX2 = SCREEN_WIDTH + 30;
    if (cloudX3 < -10) cloudX3 = SCREEN_WIDTH + 60;

    // sky changes with score
    if (score < 5)
        skyPhase = 0; // day
    else if (score < 15)
        skyPhase = 1; // sunset
    else
        skyPhase = 2; // night
}
void Background::draw(int score, int best)
{
    update(score);

    // SKY (simple style change)
    if (skyPhase == 0)
    {
        // DAY → normal clear screen
        view_render.fillScreen(BLACK);
    }
    else if (skyPhase == 1)
    {
        // SUNSET → slightly “busy” feel (dots)
        view_render.fillScreen(BLACK);
        for (int i = 0; i < 10; i++)
            view_render.drawPixel(i * 12, 5, WHITE);
    }
    else
    {
        // NIGHT → stars
        view_render.fillScreen(BLACK);
        for (int i = 0; i < 15; i++)
            view_render.drawPixel((i * 17) % 128, (i * 11) % 64, WHITE);
    }

    // CLOUDS (simple 3-pixel blobs)
    view_render.fillRect(cloudX1, 10, 8, 3, WHITE);
    view_render.fillRect(cloudX2, 20, 8, 3, WHITE);
    view_render.fillRect(cloudX3, 12, 8, 3, WHITE);
}