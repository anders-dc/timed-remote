#include "../helpers/ir_helper.h"
#include "../helpers/time_helper.h"
#include "../timed_remote.h"
#include "timed_remote_scene.h"

static void
tick_cb(void *context)
{
	TimedRemoteApp *a = context;
	view_dispatcher_send_custom_event(a->vd, EvTick);
}

static uint32_t
countdown(const TimedRemoteApp *a)
{
	return time_hms_sec(a->h, a->m, a->s);
}

static void
redraw(TimedRemoteApp *a)
{
	uint8_t h, m, s;
	time_sec_hms(a->left, &h, &m, &s);

	char t[16];
	snprintf(t, sizeof(t), "%02d:%02d:%02d", h, m, s);

	widget_reset(a->widget);
	widget_add_string_element(a->widget, 64, 5, AlignCenter, AlignTop, FontSecondary, a->sig);
	widget_add_string_element(a->widget, 64, 25, AlignCenter, AlignTop, FontBigNumbers, t);
	if (a->repeat > 0 && a->repeat < 255)
	{
		char r[24];
		snprintf(r, sizeof(r), "Repeat: %d/%d", a->repeat - a->repeat_left + 1, a->repeat);
		widget_add_string_element(a->widget, 64, 52, AlignCenter, AlignTop, FontSecondary, r);
	} else if (a->repeat == 255)
	{
		widget_add_string_element(
					a->widget, 64, 52, AlignCenter, AlignTop, FontSecondary, "Repeat: Unlimited");
	}
}

void
scene_run_enter(void *context)
{
	TimedRemoteApp *a = context;

	if (a->mode == ModeDown)
	{
		a->left = countdown(a);
	} else
	{
		a->left = time_until(a->h, a->m, a->s);
		if (a->left == 0)
		{
			view_dispatcher_send_custom_event(a->vd, EvFire);
			return;
		}
	}

	if (a->repeat == 0)
		a->repeat_left = 1;

	redraw(a);
	view_dispatcher_switch_to_view(a->vd, ViewRun);

	a->timer = furi_timer_alloc(tick_cb, FuriTimerTypePeriodic, a);
	if (!a->timer)
		return;
	furi_timer_start(a->timer, furi_kernel_get_tick_frequency());
}

bool
scene_run_event(void *context, SceneManagerEvent event)
{
	TimedRemoteApp *a = context;
	if (event.type == SceneManagerEventTypeBack)
	{
		scene_manager_search_and_switch_to_previous_scene(a->sm, ScBrowse);
		return true;
	}
	if (event.type != SceneManagerEventTypeCustom)
		return false;
	if (event.event == EvTick)
	{
		if (a->left > 0)
		{
			a->left--;
			redraw(a);
		}
		if (a->left == 0)
			view_dispatcher_send_custom_event(a->vd, EvFire);
		return true;
	}
	if (event.event != EvFire)
		return false;

	if (a->ir)
		ir_tx(a->ir);

	if (a->repeat == 255)
	{
		a->left = countdown(a);
		redraw(a);
		return true;
	}

	if (a->repeat_left > 0)
		a->repeat_left--;
	if (a->repeat != 0 && a->repeat_left > 0)
	{
		a->left = countdown(a);
		redraw(a);
		return true;
	}

	scene_manager_next_scene(a->sm, ScDone);
	return true;
}

void
scene_run_exit(void *context)
{
	TimedRemoteApp *a = context;
	if (a->timer)
	{
		furi_timer_stop(a->timer);
		furi_timer_free(a->timer);
		a->timer = NULL;
	}
	widget_reset(a->widget);
}
