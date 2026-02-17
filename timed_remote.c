#include "timed_remote.h"
#include "scenes/timed_remote_scene.h"

extern const SceneManagerHandlers scene_handlers;

static bool
nav_cb(void *context)
{
	TimedRemoteApp *app = context;
	return scene_manager_handle_back_event(app->sm);
}

static bool
evt_cb(void *context, uint32_t evt)
{
	TimedRemoteApp *app = context;
	return scene_manager_handle_custom_event(app->sm, evt);
}

TimedRemoteApp *
timed_remote_app_alloc(void)
{
	TimedRemoteApp *app = malloc(sizeof(TimedRemoteApp));
	memset(app, 0, sizeof(TimedRemoteApp));

	app->gui = furi_record_open(RECORD_GUI);

	app->vd = view_dispatcher_alloc();
	view_dispatcher_set_event_callback_context(app->vd, app);
	view_dispatcher_set_navigation_event_callback(app->vd, nav_cb);
	view_dispatcher_set_custom_event_callback(app->vd, evt_cb);
	view_dispatcher_attach_to_gui(app->vd, app->gui,
				      ViewDispatcherTypeFullscreen);

	app->sm = scene_manager_alloc(&scene_handlers, app);

	app->submenu = submenu_alloc();
	view_dispatcher_add_view(app->vd, ViewMenu,
				 submenu_get_view(app->submenu));

	app->vlist = variable_item_list_alloc();
	view_dispatcher_add_view(
		      app->vd, ViewList,
		      variable_item_list_get_view(app->vlist));

	app->widget = widget_alloc();
	view_dispatcher_add_view(app->vd, ViewRun,
				 widget_get_view(app->widget));

	app->popup = popup_alloc();
	view_dispatcher_add_view(app->vd, ViewPop,
				 popup_get_view(app->popup));

	return app;
}

void
timed_remote_app_free(TimedRemoteApp *app)
{
	if (app->timer)
	{
		furi_timer_stop(app->timer);
		furi_timer_free(app->timer);
	}

	if (app->ir)
	{
		infrared_signal_free(app->ir);
	}

	view_dispatcher_remove_view(app->vd, ViewMenu);
	submenu_free(app->submenu);

	view_dispatcher_remove_view(app->vd,
				    ViewList);
	variable_item_list_free(app->vlist);

	view_dispatcher_remove_view(app->vd, ViewRun);
	widget_free(app->widget);

	view_dispatcher_remove_view(app->vd, ViewPop);
	popup_free(app->popup);

	scene_manager_free(app->sm);

	view_dispatcher_free(app->vd);

	furi_record_close(RECORD_GUI);

	free(app);
}

int32_t
timed_remote_app(void *p)
{
	UNUSED(p);

	TimedRemoteApp *app = timed_remote_app_alloc();

	scene_manager_next_scene(app->sm, ScBrowse);

	view_dispatcher_run(app->vd);

	timed_remote_app_free(app);

	return 0;
}
