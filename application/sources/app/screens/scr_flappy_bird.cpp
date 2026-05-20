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
#define pillar_height 22

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
static int pillar_gap = 50; /* gap height between top and bottom pillar */
static int pillar_x = pillar_start_x;
static int prev_pillar_x = pillar_x;
static int pillar_speed = 1;
static int score = 0;
static int best_score = 0;
static int game_over = 0;
static ar_game_score_t flappy_scores;

static void view_scr_flappy_bird();
static void flappy_score_save();
static void flappy_load_settings();

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
    /* Draw bottom pillar from the bottom of screen upwards */
    int bottom_pillar_y = SCREEN_HEIGHT - pillar_height;
    view_render.fillRect(pillar_x, bottom_pillar_y, pillar_width, pillar_height, WHITE);
    view_render.fillRect(pillar_x - 3, bottom_pillar_y, pillar_width + 6, 3, WHITE);
}

// The top pillar for the bird to passs trhought
void flappy_bird_pilliar_top()
{
    /* Draw top pillar from top, with a gap in the middle for the bird to pass through */
    int top_pillar_height = SCREEN_HEIGHT - pillar_height - pillar_gap;
    view_render.fillRect(pillar_x, pillar_start_y, pillar_width, top_pillar_height, WHITE);
    view_render.fillRect(pillar_x - 3, top_pillar_height - 3, pillar_width + 6, 3, WHITE);
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
        }
    }

    if (pillar_x < -pillar_width)
    {
        pillar_x = SCREEN_WIDTH;
        /* randomize gap a little without extra dependencies */
        int delta = ((score % 3) - 1) * 3; /* -3, 0, +3 */
        // Pillar gap size
        int new_gap = pillar_gap + delta;
        if (new_gap < 45)
            new_gap = 45;
        if (new_gap > 55)
            new_gap = 55;
        pillar_gap = new_gap;
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
        /* Gap is in the middle: from top_pillar_height to top_pillar_height + pillar_gap */
        int top_pillar_height = SCREEN_HEIGHT - pillar_height - pillar_gap;
        int gap_top = top_pillar_height;
        int gap_bottom = top_pillar_height + pillar_gap;
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
}

static void flappy_load_settings()
{
    APP_DBG_SIG("Load settings\n");
    ar_game_setting_read(&settingdata);
    pillar_speed = settingdata.meteoroid_speed;
    if (pillar_speed < AR_GAME_SETTING_METEOROID_SPEED_MIN)
    {
        pillar_speed = AR_GAME_SETTING_METEOROID_SPEED_MIN;
    }
    if (pillar_speed > AR_GAME_SETTING_METEOROID_SPEED_MAX)
    {
        pillar_speed = AR_GAME_SETTING_METEOROID_SPEED_MAX;
    }
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

    view_render.clear();
    view_render.fillScreen(BLACK);
    /* Draw pillars */
    flappy_bird_pilliar_below();
    flappy_bird_pilliar_top();
    /* Draw bird */
    view_render.drawBitmap(bird_x, bird_y, bitmap_flappy_bird, bird_width, bird_height, 1);
    /* Draw score */
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(0, 0);
    view_render.print("SCORE:");
    view_render.print(score);
    view_render.setCursor(78, 0);
    view_render.print("BEST:");
    view_render.print(best_score);
    if (game_over)
    {
        view_render.clear();
        view_render.setCursor(7, 40);
        view_render.setTextSize(2);
        view_render.print("GAME OVER");
        view_render.drawBitmap((SCREEN_WIDTH / 2) - 8, (SCREEN_HEIGHT / 2) - 8, icon_restart, 15, 15, 0);
    }
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
        /* start fast periodic flappy update (60 FPS = 16ms) for responsive gameplay */
        timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK, 16, TIMER_PERIODIC);
        view_scr_flappy_bird();
        break;
    }

    case SCREEN_EXIT:
    {
        /* stop periodic updates when leaving screen */
        timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
        break;
    }

    case AC_DISPLAY_BUTTON_UP_RELEASED:
    {
        APP_DBG_SIG("Press Up Button\n");
        if (game_over)
        {
            /* restart game */
            game_over = 0;
            score = 0;
            bird_x = bird_start_x;
            bird_y = bird_start_y;
            bird_vy = 0;
            pillar_x = pillar_start_x;
            flappy_load_settings();
        }
        else
        {
            flappy_bird_flap();

            break;
        }

    case AC_DISPLAY_BUTTON_DOWN_RELEASED:
    {
        APP_DBG_SIG("Press Down Button\n");
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
        break;
    }

    case AC_DISPLAY_BUTTON_MODE_RELEASED:
    {
        APP_DBG_SIG("Press Mode Button\n");
        if (game_over)
        {
            SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
        }
        else
        {
            SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
        }
        break;
    }

    case AC_DISPLAY_FLAPPY_TICK:
    {
        /* periodic update tick */
        view_scr_flappy_bird();
        break;
    }

    default:
        break;
    }
    }
}