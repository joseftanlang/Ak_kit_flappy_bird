#include "button.h"

#include "sys_dbg.h"
#include "timer.h"

#include "app.h"
#include "app_bsp.h"
#include "app_dbg.h"
#include "app_if.h"
#include "screen_manager.h"
#include "scr_flappy_bird.h"

#include "task_list.h"


button_t btn_mode;
button_t btn_up;
button_t btn_down;
// bool btn_mode_state = false;

static uint8_t mode_button_target_task = AC_TASK_DISPLAY_ID;
static uint8_t up_button_target_task = AC_TASK_DISPLAY_ID;
static uint8_t down_button_target_task = AC_TASK_DISPLAY_ID;

static inline bool flappy_bird_screen_active() {
	return (scr_mng_get_current_screen() == scr_flappy_bird_handle);
}

static inline uint8_t active_button_task_id() {
	if (flappy_bird_screen_active()) {
		return AC_BIRD_DISPLAY_ID;
	}

	return AC_TASK_DISPLAY_ID;
}

static inline void post_button_event_to_task(uint8_t task_id, uint8_t sig) {
	task_post_pure_msg(task_id, sig);
}

void btn_mode_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_PRESSED\n", __func__);
		mode_button_target_task = active_button_task_id();
		post_button_event_to_task(mode_button_target_task, AC_DISPLAY_BUTTON_MODE_PRESSED);
	} break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_LONG_PRESSED\n", __func__);
		post_button_event_to_task(mode_button_target_task, AC_DISPLAY_BUTTON_MODE_LONG_PRESSED);
	} break;

	case BUTTON_SW_STATE_RELEASED: {
		APP_DBG("[%s] BUTTON_SW_STATE_RELEASED\n", __func__);
		post_button_event_to_task(mode_button_target_task, AC_DISPLAY_BUTTON_MODE_RELEASED);
		mode_button_target_task = AC_TASK_DISPLAY_ID;

		// Reset timer show idle screen
		timer_set(	AC_TASK_DISPLAY_ID, \
					AC_DISPLAY_SHOW_IDLE, \
					AC_DISPLAY_IDLE_INTERVAL, \
					TIMER_ONE_SHOT);
	} break;

	default:
		break;
	}
}

void btn_up_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_PRESSED\n", __func__);
		up_button_target_task = active_button_task_id();
		post_button_event_to_task(up_button_target_task, AC_DISPLAY_BUTTON_UP_PRESSED);
	} break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_LONG_PRESSED\n", __func__);
		post_button_event_to_task(up_button_target_task, AC_DISPLAY_BUTTON_UP_LONG_PRESSED);
		// if (btn_mode_state == true) {
		// 	task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_LONG&MODE_PRESSED);
		// }
	} break;

	case BUTTON_SW_STATE_RELEASED: {
		APP_DBG("[%s] BUTTON_SW_STATE_RELEASED\n", __func__);
		post_button_event_to_task(up_button_target_task, AC_DISPLAY_BUTTON_UP_RELEASED);
		up_button_target_task = AC_TASK_DISPLAY_ID;
		// Reset timer show idle screen
		timer_set(	AC_TASK_DISPLAY_ID, \
					AC_DISPLAY_SHOW_IDLE, \
					AC_DISPLAY_IDLE_INTERVAL, \
					TIMER_ONE_SHOT);
	} break;

	default:
		break;
	}
}

void btn_down_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_PRESSED\n", __func__);
		down_button_target_task = active_button_task_id();
		post_button_event_to_task(down_button_target_task, AC_DISPLAY_BUTTON_DOWN_PRESSED);
	} break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_LONG_PRESSED\n", __func__);
		post_button_event_to_task(down_button_target_task, AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED);
	}	
		break;

	case BUTTON_SW_STATE_RELEASED: {
		APP_DBG("[%s] BUTTON_SW_STATE_RELEASED\n", __func__);
		post_button_event_to_task(down_button_target_task, AC_DISPLAY_BUTTON_DOWN_RELEASED);
		down_button_target_task = AC_TASK_DISPLAY_ID;
		// Reset timer show idle screen
		timer_set(	AC_TASK_DISPLAY_ID, \
					AC_DISPLAY_SHOW_IDLE, \
					AC_DISPLAY_IDLE_INTERVAL, \
					TIMER_ONE_SHOT);
	} break;

	default:
		break;
	}
}
