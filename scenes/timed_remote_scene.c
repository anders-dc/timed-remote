#include "timed_remote_scene.h"

static void (*const enter[])(void *) = {
	[ScBrowse] = scene_browse_enter,
	[ScSelect] = scene_select_enter,
	[ScCfg] = scene_cfg_enter,
	[ScRun] = scene_run_enter,
	[ScDone] = scene_done_enter,
};

static bool (*const event[])(void *, SceneManagerEvent) = {
	[ScBrowse] = scene_browse_event,
	[ScSelect] = scene_select_event,
	[ScCfg] = scene_cfg_event,
	[ScRun] = scene_run_event,
	[ScDone] = scene_done_event,
};

static void (*const on_exit[])(void *) = {
	[ScBrowse] = scene_browse_exit,
	[ScSelect] = scene_select_exit,
	[ScCfg] = scene_cfg_exit,
	[ScRun] = scene_run_exit,
	[ScDone] = scene_done_exit,
};

const SceneManagerHandlers scene_handlers = {
	.on_enter_handlers = enter,
	.on_event_handlers = event,
	.on_exit_handlers = on_exit,
	.scene_num = ScN,
};
