#include "../helpers/ir_helper.h"
#include "../timed_remote.h"
#include "timed_remote_scene.h"

#define IR_DIR "/ext/infrared"

static FuriString **files;
static size_t nfiles;

static void
pick_file(void *context, uint32_t i)
{
	TimedRemoteApp *a = context;
	if (i >= nfiles)
		return;
	snprintf(a->file, sizeof(a->file), "%s/%s", IR_DIR, furi_string_get_cstr(files[i]));
	view_dispatcher_send_custom_event(a->vd, EvFile);
}

void
scene_browse_enter(void *context)
{
	TimedRemoteApp *a = context;
	submenu_reset(a->submenu);
	submenu_set_header(a->submenu, "Select IR File");

	if (ir_files(IR_DIR, &files, &nfiles))
	{
		if (nfiles == 0)
		{
			submenu_add_item(a->submenu, "(No IR files found)", 0, NULL, NULL);
		} else
		{
			for (size_t i = 0; i < nfiles; i++)
				submenu_add_item(a->submenu, furi_string_get_cstr(files[i]), i, pick_file, a);
		}
	} else
	{
		submenu_add_item(a->submenu, "(Error reading directory)", 0, NULL, NULL);
	}

	view_dispatcher_switch_to_view(a->vd, ViewMenu);
}

bool
scene_browse_event(void *context, SceneManagerEvent event)
{
	TimedRemoteApp *a = context;
	if (event.type != SceneManagerEventTypeCustom || event.event != EvFile)
		return false;
	scene_manager_next_scene(a->sm, ScSelect);
	return true;
}

void
scene_browse_exit(void *context)
{
	TimedRemoteApp *a = context;
	submenu_reset(a->submenu);
	if (!files)
		return;
	ir_files_free(files, nfiles);
	files = NULL;
	nfiles = 0;
}
