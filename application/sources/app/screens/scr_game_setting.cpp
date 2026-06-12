#include "scr_game_setting.h"

static uint8_t setting_index = 0;

static void view_scr_game_setting();

view_dynamic_t dyn_view_item_game_setting = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_setting};

view_screen_t scr_game_setting = {
	&dyn_view_item_game_setting,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

// main view function to render the game settings screen
static void view_scr_game_setting()
{
	view_render.clear();
	view_render.setTextColor(WHITE);
	view_render.setTextSize(2);
	view_render.setCursor(12, 2);
	view_render.print("SETTING");

	view_render.setTextSize(1);
	view_render.setCursor(12, 22);
	view_render.print("Sound:");
	view_render.print(settingdata.silent ? "Off" : "On");

	view_render.setCursor(12, 36);
	view_render.print("Bird Speed:");
	view_render.print(settingdata.meteoroid_speed);

	// highlight the selected setting
	if (setting_index == 0)
	{
		view_render.drawRect(6, 19, 116, 12, WHITE);
	}
	else
	{
		view_render.drawRect(6, 33, 116, 12, WHITE);
	}

	view_render.setCursor(10, 54);
	view_render.print("MODE save");
	view_render.update();
}

// save settings to EEPROM and return to menu screen
static void setting_save_and_exit()
{
	ar_game_setting_write(&settingdata);
	scr_idle_set_return_screen(scr_menu_game_handle, &scr_menu_game);
	SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
}

// handle screen events like button presses and screen entry/exit
void scr_game_setting_handle(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");
		view_render.initialize();
		view_render_display_on();
		ar_game_setting_read(&settingdata);
		setting_index = 0;
	}
	break;

	case AC_DISPLAY_BUTTON_UP_RELEASED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
		setting_index = 0;
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	}
	break;

	case AC_DISPLAY_BUTTON_DOWN_RELEASED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
		setting_index = 1;
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	}
	break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		// toggle sound setting or cycle through speed settings based on which setting is selected
		if (setting_index == 0)
		{
			settingdata.silent = !settingdata.silent;
			BUZZER_Sleep(settingdata.silent);
		}
		else
		{
			settingdata.meteoroid_speed++;
			if (settingdata.meteoroid_speed > AR_GAME_SETTING_METEOROID_SPEED_MAX)
			{
				settingdata.meteoroid_speed = AR_GAME_SETTING_METEOROID_SPEED_MIN;
			}
		}
		ar_game_setting_write(&settingdata);
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	}
	break;

	case AC_DISPLAY_BUTTON_UP_LONG_PRESSED:
	case AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED:
	case AC_DISPLAY_BUTTON_MODE_LONG_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_LONG_PRESSED\n");
		setting_save_and_exit();
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	}
	break;

	default:
		break;
	}
}