#include "scr_flappy_bird.h"
#include "screen_manager.h"
#include "app_eeprom.h"

// screen width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// The pillar for the bird to passs trhought, the gap between the top and bottom pillar is 30 pixels.
#define pillar_start_x 90
#define pillar_start_y 0
#define pillar_width 10
#define pillar_type_count 3

// The Bird image and actions
#define bird_height 15
#define bird_width 15
#define bird_start_x 10
#define bird_start_y 25

// The game enviroment
#define gravity 2
#define velocity 0
#define jump_height_up_button 8 // when the user presses the up button, bird velocity is set to this (upwards)

/* Game state globals (simple implementation) */
static int bird_x = bird_start_x;
static int bird_y = bird_start_y;
static int bird_vy = 0;
static int pillar_gap = 50;    /* gap height between top and bottom pillar */
static int pillar_gap_top = 7; /* y-position where the gap starts */
static int pillar_x = pillar_start_x;
static int prev_pillar_x = pillar_x;
static int pillar_type = 0;
static int pillar_speed_base = 1;
static int pillar_speed = 1;
static int score = 0;
static int best_score = 0;
static int game_over = 0;
static ar_game_score_t flappy_scores;

static void view_scr_flappy_bird();
static void flappy_score_save();
static void flappy_load_settings();
static void flappy_set_pillar_variant(int type_idx);
static void flappy_next_pillar_variant();
static void flappy_apply_difficulty();

view_dynamic_t dyn_view_item_flappy_bird = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_flappy_bird};

view_screen_t scr_flappy_bird = {
    &dyn_view_item_flappy_bird,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

// The bottom pillar for the bird to passs trhought
void flappy_bird_pilliar_below()
{
    /* Draw bottom pillar from gap bottom to the bottom of screen */
    int bottom_pillar_y = pillar_gap_top + pillar_gap;
    int bottom_pillar_height = SCREEN_HEIGHT - bottom_pillar_y;
    if (bottom_pillar_height > 0)
    {
        view_render.fillRect(pillar_x, bottom_pillar_y, pillar_width, bottom_pillar_height, WHITE);
        view_render.fillRect(pillar_x - 3, bottom_pillar_y, pillar_width + 6, 3, WHITE);
    }
}

// The top pillar for the bird to passs trhought
void flappy_bird_pilliar_top()
{
    /* Draw top pillar from top to gap start */
    int top_pillar_height = pillar_gap_top;
    view_render.fillRect(pillar_x, pillar_start_y, pillar_width, top_pillar_height, WHITE);
    view_render.fillRect(pillar_x - 3, top_pillar_height - 3, pillar_width + 6, 3, WHITE);
}

static void flappy_set_pillar_variant(int type_idx)
{
    static const int gap_size[pillar_type_count] = {42, 48, 38};
    static const int gap_top[pillar_type_count] = {6, 14, 22};
    static const int spawn_offset_x[pillar_type_count] = {0, 10, 18};

    if (type_idx < 0)
    {
        type_idx = 0;
    }
    pillar_type = type_idx % pillar_type_count;

    pillar_gap = gap_size[pillar_type];
    pillar_gap_top = gap_top[pillar_type];
    pillar_x = SCREEN_WIDTH + spawn_offset_x[pillar_type];
    prev_pillar_x = pillar_x;
}

static void flappy_next_pillar_variant()
{
    /* deterministic rotate by score => 3 distinct repeating styles */
    flappy_set_pillar_variant(score % pillar_type_count);
}

static void flappy_apply_difficulty()
{
    int challenge_speed = pillar_speed_base + (score / 2);

    if (challenge_speed < AR_GAME_SETTING_METEOROID_SPEED_MIN)
    {
        challenge_speed = AR_GAME_SETTING_METEOROID_SPEED_MIN;
    }
    if (challenge_speed > AR_GAME_SETTING_METEOROID_SPEED_MAX)
    {
        challenge_speed = AR_GAME_SETTING_METEOROID_SPEED_MAX;
    }

    pillar_speed = challenge_speed;
}

static void update_bird()
{
    /* apply gravity */
    bird_vy += gravity;
    bird_y += bird_vy;
    /* clamp */
    if (bird_y < 0)
        bird_y = 0;
    if (bird_y > SCREEN_HEIGHT - bird_height)
        bird_y = SCREEN_HEIGHT - bird_height;
}

static void update_pillar()
{
    /* move pillar left; wrap when off-screen */
    prev_pillar_x = pillar_x;
    pillar_x -= pillar_speed;
    /* scoring: when pillar passes bird x, increment */
    if (prev_pillar_x >= (bird_x + bird_width) && pillar_x < (bird_x + bird_width))
    {
        if (!game_over)
        {
            score++;
            if (score > best_score)
                best_score = score;
            flappy_apply_difficulty();
        }
    }

    if (pillar_x < -pillar_width)
    {
        flappy_next_pillar_variant();
    }
}

static void check_collision()
{
    if (game_over)
        return;
    /* check horizontal overlap */
    int bird_left = bird_x;
    int bird_right = bird_x + bird_width;
    int pillar_left = pillar_x;
    int pillar_right = pillar_x + pillar_width;

    if (bird_right > pillar_left && bird_left < pillar_right)
    {
        /* within pillar x-range; check gap */
        int gap_top = pillar_gap_top;
        int gap_bottom = pillar_gap_top + pillar_gap;
        int bird_top = bird_y;
        int bird_bottom = bird_y + bird_height;
        if (bird_top < gap_top || bird_bottom > gap_bottom)
        {
            /* collision */
            game_over = 1;
            bird_vy = 0;
            flappy_score_save();
        }
    }
    // also check collision with top and bottom of screen (ground and ceiling)
    if (bird_y <= 0 || bird_y + bird_height >= SCREEN_HEIGHT)
    {
        game_over = 1;
    }
}

static void flappy_load_settings()
{
    APP_DBG_SIG("Load settings\n");
    ar_game_setting_read(&settingdata);
    pillar_speed_base = settingdata.meteoroid_speed;
    if (pillar_speed_base < AR_GAME_SETTING_METEOROID_SPEED_MIN)
    {
        pillar_speed_base = AR_GAME_SETTING_METEOROID_SPEED_MIN;
    }
    if (pillar_speed_base > AR_GAME_SETTING_METEOROID_SPEED_MAX)
    {
        pillar_speed_base = AR_GAME_SETTING_METEOROID_SPEED_MAX;
    }

    flappy_apply_difficulty();
}

static void flappy_score_save()
{
    APP_DBG_SIG("score saved\n");
    ar_game_score_read(&flappy_scores);
    flappy_scores.score_now = (uint32_t)score;

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

static void view_scr_flappy_bird()
{
    /* update physics/state on each render call */
    if (!game_over)
    {
        update_bird();
        update_pillar();
        check_collision();
    }

    view_render.fillScreen(BLACK);
    /* Draw pillars */
    flappy_bird_pilliar_below();
    flappy_bird_pilliar_top();
    /* Draw bird */
    view_render.drawBitmap(bird_x, bird_y, bitmap_flappy_bird, bird_width, bird_height, 1);
    /* Draw score */
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(8, 2);
    view_render.print("SCORE:");
    view_render.print(score);
    view_render.setCursor(78, 2);
    view_render.print("BEST:");
    view_render.print(best_score);
    view_render.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 7, WHITE);
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
        view_render.setTextSize(1);
        view_render.print("Press MODE to Score");
        view_render.setCursor(7, 46);
        view_render.setTextSize(1);
        view_render.print("Press UP to Reset");
    }
    view_render.update();
}

// To make the bird keep flapping up to stay in the air, if never press up button the bird will slwoly go down.
static void flappy_bird_flap()
{
    /* give bird an upwards velocity */
    bird_vy = -jump_height_up_button;
}

void scr_flappy_bird_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
    {
        APP_DBG_SIG("SCREEN_ENTRY\n");
        flappy_load_settings();
        ar_game_score_read(&flappy_scores);
        best_score = flappy_scores.score_1st;
        flappy_set_pillar_variant(0);
        timer_set(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK, AC_DISPLAY_MINIMUM_SCREEN_RENDER_INTERVAL_MS, TIMER_PERIODIC);
        break;
    }

    case SCREEN_EXIT:
    {
        APP_DBG_SIG("FLAPPY SCREEN_EXIT\n");

        timer_remove_attr(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);

        game_over = 0;
        bird_vy = 0;

        view_render_display_off();

        break;
    }

    case AC_DISPLAY_BUTTON_UP_PRESSED:
    {
        APP_DBG_SIG("Press Up Button\n");
        if (game_over == 1)
        {
            /* restart game */
            game_over = 0;
            score = 0;
            bird_x = bird_start_x;
            bird_y = bird_start_y;
            bird_vy = 0;
            flappy_set_pillar_variant(0);
            flappy_load_settings();
        }
        else
        {
            flappy_bird_flap();
        }
        break;
    }

    case AC_DISPLAY_BUTTON_DOWN_PRESSED:
    {
        APP_DBG("Before SCREEN_TRAN\n");
        if (game_over == 1)
        {
            timer_remove_attr(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
            view_render_display_off();
            game_over = 0;
            SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
            APP_DBG("Before SCREEN_TRAN\n");
        }
        break;
    }

    case AC_DISPLAY_BUTTON_MODE_PRESSED:
    {
        APP_DBG_SIG("Press Mode Button\n");
        if (game_over == 1)
        {   
            game_over = 0;
            timer_remove_attr(AC_BIRD_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
            view_render_display_off();
            SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
        }
        break;
    }

    case AC_DISPLAY_FLAPPY_TICK:
    {
        /* periodic update tick: rendering is handled by screen manager dispatch */

        break;
    }

    default:
        break;
    }
}