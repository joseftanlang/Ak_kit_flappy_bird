#include "scr_startup.h"

#define STARTUP_ANIM_SIG           (AR_GAME_DEFINE_SIG + 40)
#define STARTUP_ANIM_INTERVAL_MS   (80)

static uint8_t startup_anim_frame = 0;
static uint8_t startup_phase = 0; // 0=bg,1=logo-in,2=particles,3=title,4=idle
static uint8_t phase_counter = 0;
static uint8_t logo_settled = 0;
static uint8_t logo_bounce = 0;

// simple particle sparkle (smaller, unobtrusive)
#define STARTUP_PARTICLES 6
typedef struct { int8_t x; int8_t y; int8_t vy; int8_t life; } particle_t;
static particle_t particles[STARTUP_PARTICLES];

static void view_scr_startup();

view_dynamic_t dyn_view_startup = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_startup
};

view_screen_t scr_startup = {
	&dyn_view_startup,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_startup() {
#define TITLE_X    28
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	// simple noisy sky background (subtle)
	for (int y = 0; y < 64; y++) {
		for (int x = 0; x < 128; x += 8) {
			int v = (x * 7) ^ (y * 3) ^ (startup_anim_frame);
			if ((v & 63) == 0) view_render.drawPixel(x + ((v >> 3) & 7), y, WHITE);
		}
	}

	// moving pipes (parallax) - non-interactive animation for startup
	const int p_w = 10;
	// create a few pipe columns with offsets based on frame
	for (int i = 0; i < 3; i++) {
		int speed = 1 + (i & 1); // slight speed variation
		int base_x = 128 - ((startup_anim_frame * speed) % 150) + i * 48;
		int gap = 36 + ((i * 7 + startup_anim_frame) % 12); // vary gap a bit
		int top_h = 8 + ((i * 3 + startup_anim_frame) % 18);
		// only draw if on screen
		if (base_x > -p_w && base_x < 128) {
			view_render.fillRect(base_x, 0, p_w, top_h, WHITE);
			view_render.fillRect(base_x - 3, top_h - 3, p_w + 6, 3, WHITE);
			int bottom_y = top_h + gap;
			view_render.fillRect(base_x, bottom_y, p_w, 64 - bottom_y, WHITE);
			view_render.fillRect(base_x - 3, bottom_y, p_w + 6, 3, WHITE);
		}
	}

	// bird bob + simple wing-flap effect (vertical oscillation)
	extern const unsigned char PROGMEM bitmap_flappy_bird[];
	int bird_x = 18;
	int bob = ((startup_anim_frame & 7) < 4) ? -1 : 1;
	int bird_y = 22 + ((startup_anim_frame >> 2) & 3) - 1 + bob;
	view_render.drawBitmap(80, bird_y + 5, bitmap_flappy_bird, 15, 15, 1);

	// big title: FLAPPY BIRD with pulse when logo settles
	int pulse = (startup_anim_frame >> 2) & 3;
	int title_y = 6 + ((pulse == 1) ? 1 : 0);
	view_render.setTextSize(2);
	view_render.setCursor(TITLE_X, title_y);
	view_render.print("FLAPPY");
	view_render.setTextSize(1);
	view_render.setCursor(TITLE_X + 2, title_y + 18);
	view_render.print("BIRD");

	// subtitle / hint
	static const char* subtitle = "Press to start";
	int reveal = (startup_phase >= 3) ? (phase_counter / 1) : (startup_anim_frame / 4);
	if (reveal > (int)strlen(subtitle)) reveal = strlen(subtitle);
	char buf[32];
	memcpy(buf, subtitle, reveal);
	buf[reveal] = '\0';
	view_render.setTextSize(1);
	view_render.setCursor(14, 54);
	view_render.print(buf);

	view_render.update();
}
void scr_startup_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_DISPLAY_INITIAL: {
		APP_DBG_SIG("AC_DISPLAY_INITIAL\n");
		view_render.initialize();
		view_render_display_on();
		startup_anim_frame = 0;
		// initialize particles
		for (int i = 0; i < STARTUP_PARTICLES; i++) {
			particles[i].x = (int8_t)(8 + (i * 9) % 110); // spread across width
			particles[i].y = (int8_t)(10 + (i * 5) % 40); // start in upper half
			particles[i].vy = (int8_t)(1 + (i % 3)); // slow downward speed
			particles[i].life = (int8_t)(i % 6 + 4); // staggered initial life
		}
		startup_phase = 0;
		phase_counter = 0;
		timer_set(AC_TASK_DISPLAY_ID, STARTUP_ANIM_SIG, STARTUP_ANIM_INTERVAL_MS, TIMER_PERIODIC);
		timer_set(	AC_TASK_DISPLAY_ID, \
					AC_DISPLAY_SHOW_STARTUP_LOGO, \
					AC_DISPLAY_STARTUP_INTERVAL, \
					TIMER_ONE_SHOT);
		// Read setting
		ar_game_setting_read(&settingdata);
		BUZZER_Sleep(settingdata.silent);
		BUZZER_PlaySound(BUZZER_SOUND_STARTUP);
	} break;

	case STARTUP_ANIM_SIG: {
		startup_anim_frame++;
		// update particles: move active ones, respawn inactive ones smoothly
		for (int i = 0; i < STARTUP_PARTICLES; i++) {
			if (particles[i].life > 0) {
				particles[i].y += particles[i].vy;
				particles[i].life--;
				if (particles[i].y > 58) particles[i].life = 0; // mark dead to respawn
			} else {
				// sparser respawn to make them less intrusive
				if (((startup_anim_frame + i * 11) & 7) == 0) {
					particles[i].x = (int8_t)(24 + ((i * 31 + startup_anim_frame * 5) % 80));
					particles[i].y = (int8_t)(6 + ((i * 13 + startup_anim_frame) % 10));
					particles[i].vy = 1; // slow
					particles[i].life = (int8_t)(2 + (i % 4));
				}
			}
		}
		// phase progression (timed by frames)
		if (startup_phase == 0 && startup_anim_frame > 8) { startup_phase = 1; phase_counter = 1; }
		if (startup_phase == 1) {
			phase_counter++;
			if (phase_counter > 16) startup_phase = 2;
		}
		if (startup_phase == 2) {
			phase_counter++;
			if (phase_counter > 18) startup_phase = 3;
		}
		if (startup_phase == 3) {
			phase_counter++;
			if (phase_counter > 28) startup_phase = 4;
		}
		// decay logo bounce counter
		if (logo_bounce) logo_bounce--;
	} break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		// user pressed a button -> stop animation and transition
		timer_remove_attr(AC_TASK_DISPLAY_ID, STARTUP_ANIM_SIG);
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_BUTTON_UP_RELEASED:
	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP/DOWN_RELEASED -> transit\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, STARTUP_ANIM_SIG);
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_SHOW_STARTUP_LOGO: {
		APP_DBG_SIG("AC_DISPLAY_SHOW_STARTUP_LOGO\n");
		// Stop automatic transition; leave animation stopped and wait for user input
		timer_remove_attr(AC_TASK_DISPLAY_ID, STARTUP_ANIM_SIG);
		startup_phase = 4; // mark idle/complete — wait for user button to go next
	} break;

	default:
		break;
	}
}
