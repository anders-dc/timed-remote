#include "../timed_remote.h"
#include "timed_remote_scene.h"

static void
done_cb(void *context)
{
	TimedRemoteApp *a = context;
	view_dispatcher_send_custom_event(a->vd, EvDone);
}

void
scene_done_enter(void *context)
{
	TimedRemoteApp *a = context;
	popup_reset(a->popup);
	popup_set_header(a->popup, "Signal Sent!", 64, 20, AlignCenter, AlignCenter);
	popup_set_text(a->popup, a->sig, 64, 35, AlignCenter, AlignCenter);
	popup_set_timeout(a->popup, 2000);
	popup_set_context(a->popup, a);
	popup_set_callback(a->popup, done_cb);
	popup_enable_timeout(a->popup);
	view_dispatcher_switch_to_view(a->vd, ViewPop);
}

bool
scene_done_event(void *context, SceneManagerEvent event)
{
	TimedRemoteApp *a = context;
	if (event.type != SceneManagerEventTypeCustom || event.event != EvDone)
		return false;
	scene_manager_search_and_switch_to_previous_scene(a->sm, ScBrowse);
	return true;
}

void
scene_done_exit(void *context)
{
	TimedRemoteApp *a = context;
	popup_reset(a->popup);
}
