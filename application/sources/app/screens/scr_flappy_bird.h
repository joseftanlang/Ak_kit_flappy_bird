#ifndef __SCR_FLAPPY_BIRD_H__
#define __SCR_FLAPPY_BIRD_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include "eeprom.h"
#include "app_eeprom.h"

#include "screens.h"
#include "screens_bitmap.h"

extern void flappy_bird_pilliar_below();
extern void flappy_bird_pilliar_top();

extern view_dynamic_t dyn_view_item_flappy_bird;
extern view_screen_t scr_flappy_bird;
extern void scr_flappy_bird_handle(ak_msg_t* msg);

#endif //__SCR_FLAPPY_BIRD_H__