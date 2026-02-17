#include "../timed_remote.h"
#include "timed_remote_scene.h"

enum
{
	IdxMode,
	IdxH,
	IdxM,
	IdxS,
	IdxRepeat,
	IdxStart,
};

#define REP_OFF 0U
#define REP_INF 255U

static void
set2(VariableItem *it, uint8_t v)
{
	char s[4];
	snprintf(s, sizeof(s), "%02d", v);
	variable_item_set_current_value_text(it, s);
}

static uint8_t
rep_from_idx(uint8_t i)
{
	if (i == 0)
		return REP_OFF;
	if (i == 1)
		return REP_INF;
	return i - 1;
}

static uint8_t
idx_from_rep(uint8_t r)
{
	if (r == REP_OFF)
		return 0;
	if (r == REP_INF)
		return 1;
	return r + 1;
}

static void
set_rep(VariableItem *it, uint8_t r)
{
	if (r == REP_OFF)
	{
		variable_item_set_current_value_text(it, "Off");
		return;
	}
	if (r == REP_INF)
	{
		variable_item_set_current_value_text(it, "Unlimited");
		return;
	}
	char s[16];
	snprintf(s, sizeof(s), "%u", r);
	variable_item_set_current_value_text(it, s);
}

static uint32_t
start_idx(const TimedRemoteApp *a)
{
	if (a->mode == ModeDown)
		return IdxStart;
	return IdxRepeat;
}

static void
mode_chg(VariableItem *it)
{
	TimedRemoteApp *a = variable_item_get_context(it);
	a->mode = variable_item_get_current_value_index(it) == 0
		? ModeDown
		: ModeSched;
	variable_item_set_current_value_text(
					     it, a->mode == ModeDown ? "Countdown" : "Scheduled");
	if (a->mode == ModeSched)
		a->repeat = REP_OFF;
	view_dispatcher_send_custom_event(a->vd, EvMode);
}

static void
h_chg(VariableItem *it)
{
	TimedRemoteApp *a = variable_item_get_context(it);
	a->h = variable_item_get_current_value_index(it);
	set2(it, a->h);
}

static void
m_chg(VariableItem *it)
{
	TimedRemoteApp *a = variable_item_get_context(it);
	a->m = variable_item_get_current_value_index(it);
	set2(it, a->m);
}

static void
s_chg(VariableItem *it)
{
	TimedRemoteApp *a = variable_item_get_context(it);
	a->s = variable_item_get_current_value_index(it);
	set2(it, a->s);
}

static void
rep_chg(VariableItem *it)
{
	TimedRemoteApp *a = variable_item_get_context(it);
	a->repeat = rep_from_idx(variable_item_get_current_value_index(it));
	set_rep(it, a->repeat);
}

static void
enter_cb(void *ctx, uint32_t i)
{
	TimedRemoteApp *a = ctx;
	if (i != start_idx(a))
		return;
	view_dispatcher_send_custom_event(a->vd, EvCfg);
}

static void
add_tm(VariableItemList *l, const char *n, uint8_t nvals, uint8_t v, VariableItemChangeCallback cb, TimedRemoteApp *a)
{
	VariableItem *it = variable_item_list_add(l, n, nvals, cb, a);
	variable_item_set_current_value_index(it, v);
	set2(it, v);
}

static void
build(TimedRemoteApp *a)
{
	variable_item_list_reset(a->vlist);

	VariableItem *it = variable_item_list_add(a->vlist, "Mode", 2, mode_chg, a);
	variable_item_set_current_value_index(it, a->mode == ModeDown ? 0 : 1);
	variable_item_set_current_value_text(
					     it, a->mode == ModeDown ? "Countdown" : "Scheduled");

	add_tm(a->vlist, "Hours", 24, a->h, h_chg, a);
	add_tm(a->vlist, "Minutes", 60, a->m, m_chg, a);
	add_tm(a->vlist, "Seconds", 60, a->s, s_chg, a);

	if (a->mode == ModeDown)
	{
		it = variable_item_list_add(a->vlist, "Repeat", 101, rep_chg, a);
		variable_item_set_current_value_index(it, idx_from_rep(a->repeat));
		set_rep(it, a->repeat);
	}

	variable_item_list_add(a->vlist, ">> Start Timer <<", 0, NULL, NULL);
	variable_item_list_set_enter_callback(a->vlist, enter_cb, a);
}

void
scene_cfg_enter(void *context)
{
	TimedRemoteApp *a = context;
	build(a);
	view_dispatcher_switch_to_view(a->vd, ViewList);
}

bool
scene_cfg_event(void *context, SceneManagerEvent event)
{
	TimedRemoteApp *a = context;
	if (event.type != SceneManagerEventTypeCustom)
		return false;
	if (event.event == EvMode)
	{
		build(a);
		return true;
	}
	if (event.event != EvCfg)
		return false;
	if (a->repeat == REP_OFF)
	{
		a->repeat_left = 1;
	} else if (a->repeat == REP_INF)
	{
		a->repeat_left = REP_INF;
	} else
	{
		a->repeat_left = a->repeat + 1;
	}
	scene_manager_next_scene(a->sm, ScRun);
	return true;
}

void
scene_cfg_exit(void *context)
{
	TimedRemoteApp *a = context;
	variable_item_list_reset(a->vlist);
}
