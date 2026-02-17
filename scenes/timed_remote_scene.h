#pragma once

#include <gui/scene_manager.h>

typedef enum
{
	ScBrowse,
	ScSelect,
	ScCfg,
	ScRun,
	ScDone,
	ScN,
} SceneId;

typedef enum
{
	EvFile,
	EvSig,
	EvMode,
	EvCfg,
	EvTick,
	EvFire,
	EvDone,
} EvId;

void scene_browse_enter(void *context);
bool scene_browse_event(void *context, SceneManagerEvent event);
void scene_browse_exit(void *context);
void scene_select_enter(void *context);
bool scene_select_event(void *context, SceneManagerEvent event);
void scene_select_exit(void *context);
void scene_cfg_enter(void *context);
bool scene_cfg_event(void *context, SceneManagerEvent event);
void scene_cfg_exit(void *context);
void scene_run_enter(void *context);
bool scene_run_event(void *context, SceneManagerEvent event);
void scene_run_exit(void *context);
void scene_done_enter(void *context);
bool scene_done_event(void *context, SceneManagerEvent event);
void scene_done_exit(void *context);
