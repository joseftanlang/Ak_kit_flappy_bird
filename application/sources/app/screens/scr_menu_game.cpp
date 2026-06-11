#include "scr_menu_game.h"

static const char* const menu_items[] = {
	"Flappy Bird",
	"Score",
	"Setting",
	// "Flappy Bird",
};

static uint8_t menu_index = 0;

static void view_scr_menu_game();

view_dynamic_t dyn_view_menu = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_menu_game
};

view_screen_t scr_menu_game = {
	&dyn_view_menu,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

static void view_scr_menu_game() {
	view_render.clear();
	view_render.setTextSize(2);
	view_render.setTextColor(WHITE);
	view_render.setCursor(35, 1);
	view_render.print("MENU");

	view_render.setTextSize(1);
	for (uint8_t i = 0; i < 3; i++) {
		int y = 22 + (i * 10);
		if (i == menu_index) {
			view_render.fillRoundRect(6, y - 1, 116, 10, 3, WHITE);
			view_render.setTextColor(BLACK);
		}
		else {
			view_render.setTextColor(WHITE);
		}
		view_render.setCursor(14, y);
		view_render.print(menu_items[i]);
	}
	view_render.update();
}

static void menu_go_idle() {
	scr_idle_set_return_screen(scr_menu_game_handle, &scr_menu_game);
	SCREEN_TRAN(scr_idle_handle, &scr_idle);
}

static void menu_go_score() {
	SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
}

static void menu_go_setting() {
	SCREEN_TRAN(scr_game_setting_handle, &scr_game_setting);
}

static void menu_go_flappy_bird() {
	SCREEN_TRAN(scr_flappy_bird_handle, &scr_flappy_bird);
}

void scr_menu_game_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		view_render.initialize();
		view_render_display_on();
		menu_index = 0;
	} break;

	case AC_DISPLAY_BUTTON_UP_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_PRESSED\n");
		if (menu_index == 0) {
			menu_index = 2;
		}
		else {
			menu_index--;
		}
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_BUTTON_DOWN_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_PRESSED\n");
		menu_index = (menu_index + 1) % 3;
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_BUTTON_MODE_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_PRESSED\n");
		if (menu_index == 0) {
			menu_go_flappy_bird();
		}
		else if (menu_index == 1) {
			menu_go_score();
		}
		else if (menu_index == 2) {
			menu_go_setting();
		} else {
			menu_go_idle();
		}
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;


	default:
		break;
	}
}