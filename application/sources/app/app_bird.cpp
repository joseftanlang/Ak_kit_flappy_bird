#include "fsm.h"
#include "port.h"
#include "message.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"

/*
 * Simple bird task stubs.
 * - `bird_up_button` forwards input to the display/screen handler so the flappy
 *   screen receives the up button event.
 * - Other tasks are kept as no-op placeholders usable for future logic.
 */

void bird_id(ak_msg_t* msg) {
	(void)msg;
	/* display-related messages for the bird can be handled by the screen handler */
}

void bird_up_button(ak_msg_t* msg) {
	(void)msg;
	/* forward the up-button event to the display task so the active screen handles it */
	task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_RELEASED);
}

void bird_pillar(ak_msg_t* msg) {
	if (!msg) return;
	if (msg->sig == AC_BIRD_PILLAR_TICK) {
		/* tell display to perform a flappy tick (updates+render) */
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
	}
}

void bird_score(ak_msg_t* msg) {
	if (!msg) return;
	if (msg->sig == AC_BIRD_SCORE_TICK) {
		/* placeholder: could persist best score or play sound */
		/* notify display to refresh score display */
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_FLAPPY_TICK);
	}
}
