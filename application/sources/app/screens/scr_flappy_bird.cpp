#include "scr_flappy_bird.h"

#include "../game/flappy_bird_bird.h"
#include "../game/flappy_bird_pillar.h"
#include "../game/flappy_bird_background.h"

#include "screen_manager.h"
#include "app_eeprom.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

static Bird bird;
static Pillar pillar;
static Background background;

static int score = 0;
static int best_score = 0;
static int game_over = 0;

static int prev_pillar_x = 0;

static ar_game_score_t flappy_scores;

static void flappy_score_save();
static void flappy_load_settings();
static void check_collision();
static void view_scr_flappy_bird();

view_dynamic_t dyn_view_item_flappy_bird =
{
    { .item_type = ITEM_TYPE_DYNAMIC },
    view_scr_flappy_bird
};

view_screen_t scr_flappy_bird =
{
    &dyn_view_item_flappy_bird,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
};

// load game settings from EEPROM and apply to pillar speed
static void flappy_load_settings()
{
    ar_game_setting_read(&settingdata);

    int baseSpeed = settingdata.meteoroid_speed;

    if (baseSpeed < AR_GAME_SETTING_METEOROID_SPEED_MIN)
        baseSpeed = AR_GAME_SETTING_METEOROID_SPEED_MIN;

    if (baseSpeed > AR_GAME_SETTING_METEOROID_SPEED_MAX)
        baseSpeed = AR_GAME_SETTING_METEOROID_SPEED_MAX;

    pillar.baseSpeed = baseSpeed;
    pillar.speed = baseSpeed;
}

// save current score to EEPROM and update best scores if needed
static void flappy_score_save()
{
    ar_game_score_read(&flappy_scores);

    flappy_scores.score_now = score;

    if (flappy_scores.score_now > flappy_scores.score_1st)
    {
        flappy_scores.score_3rd = flappy_scores.score_2nd;
        flappy_scores.score_2nd = flappy_scores.score_1st;
        flappy_scores.score_1st = flappy_scores.score_now;
    }
    else if (flappy_scores.score_now > flappy_scores.score_2nd)
    {
        flappy_scores.score_3rd = flappy_scores.score_2nd;
        flappy_scores.score_2nd = flappy_scores.score_now;
    }
    else if (flappy_scores.score_now > flappy_scores.score_3rd)
    {
        flappy_scores.score_3rd = flappy_scores.score_now;
    }

    ar_game_score_write(&flappy_scores);
}

// check for collision between bird and pillar, or bird and ground/ceiling, and set game_over if collision occurs
static void check_collision()
{
    if (game_over)
        return;

    int birdLeft = bird.x;
    int birdRight = bird.x + Bird::BIRD_WIDTH;

    int pillarLeft = pillar.x;
    int pillarRight = pillar.x + Pillar::PILLAR_WIDTH;

    if (birdRight > pillarLeft && birdLeft < pillarRight)
    {
        int gapTop = pillar.gapTop;
        int gapBottom = pillar.gapTop + pillar.gap;

        int birdTop = bird.y;
        int birdBottom = bird.y + Bird::BIRD_HEIGHT;

        if (birdTop < gapTop || birdBottom > gapBottom)
        {
            game_over = 1;
            flappy_score_save();
        }
    }

    if (bird.y <= 0 ||
        bird.y + Bird::BIRD_HEIGHT >= SCREEN_HEIGHT)
    {
        game_over = 1;
        flappy_score_save();
    }
}

// main view function to update game state and render everything on screen
static void view_scr_flappy_bird()
{
    if (!game_over)
    {
        bird.update();

        prev_pillar_x = pillar.x;

        // pillar.applyDifficulty(score);
        pillar.speed = pillar.baseSpeed + (score / 2);
        if (pillar.speed > AR_GAME_SETTING_METEOROID_SPEED_MAX)
            pillar.speed = AR_GAME_SETTING_METEOROID_SPEED_MAX;

        pillar.update();

        // triggers exactly ONCE when pillar passes bird center
        if (prev_pillar_x >= bird.x && pillar.x < bird.x)
        {
            score++;

            if (score > best_score)
                best_score = score;
        }

        check_collision();

        // if pillar goes off screen, reset it and reload settings in case user changed difficulty
        if (pillar.x < -Pillar::PILLAR_WIDTH)
        {
            pillar.init();
            flappy_load_settings();
        }
    }

    background.draw(score, best_score);

    pillar.draw();
    bird.draw();

    // display score and best score at the top
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);

    view_render.setCursor(8, 2);
    view_render.print("Score:");
    view_render.print(score);

    view_render.setCursor(70, 2);
    view_render.print("Best:");
    view_render.print(best_score);

    // if game over, display game over message and options
    if (game_over)
    {
        view_render.clear();

        view_render.setCursor(7, 2);
        view_render.setTextSize(2);
        view_render.print("GAME OVER");

        view_render.setCursor(7, 22);
        view_render.setTextSize(1);
        view_render.print("Press DOWN to Menu");

        view_render.setCursor(7, 34);
        view_render.print("Press MODE to Score");

        view_render.setCursor(7, 46);
        view_render.print("Press UP to Reset");
    }

    view_render.update();
}

// handle screen events like button presses and screen entry/exit
void scr_flappy_bird_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
    {
        flappy_load_settings();

        ar_game_score_read(&flappy_scores);
        best_score = flappy_scores.score_1st;

        bird.init();
        pillar.init();

        score = 0;
        game_over = 0;

        prev_pillar_x = pillar.x;

        timer_set(
            AC_BIRD_DISPLAY_ID,
            AC_DISPLAY_FLAPPY_TICK,
            AC_DISPLAY_MINIMUM_SCREEN_RENDER_INTERVAL_MS,
            TIMER_PERIODIC);

        break;
    }

    case SCREEN_EXIT:
    {
        timer_remove_attr(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
        view_render_display_off();
        break;
    }

    case AC_DISPLAY_BUTTON_UP_PRESSED:
    {
        if (game_over)
        {
            bird.init();
            pillar.init();
            flappy_load_settings();

            score = 0;
            game_over = 0;
            prev_pillar_x = pillar.x;
        }
        else
        {
            bird.flap();
        }
        break;
    }

    case AC_DISPLAY_BUTTON_DOWN_PRESSED:
    {
        if (game_over)
        {
            timer_remove_attr(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
            view_render_display_off();

            SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
        }
        break;
    }
    case AC_DISPLAY_BUTTON_MODE_PRESSED:
    {
        if (game_over)
        {
            timer_remove_attr(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
            view_render_display_off();

            SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
        }
        break;
    }

    default:
        break;
    }
}