#include "../helpers/ir_helper.h"
#include "../timed_remote.h"
#include "timed_remote_scene.h"

static IrSignalList *sigs;

static void
pick_sig(void *context, uint32_t i)
{
	TimedRemoteApp *a = context;
	if (!sigs || i >= sigs->count)
		return;

	if (a->ir)
		infrared_signal_free(a->ir);
	a->ir = infrared_signal_alloc();
	if (!a->ir)
		return;

	infrared_signal_set_signal(a->ir, sigs->items[i].signal);
	strncpy(a->sig, furi_string_get_cstr(sigs->items[i].name), sizeof(a->sig) - 1);
	a->sig[sizeof(a->sig) - 1] = '\0';
	view_dispatcher_send_custom_event(a->vd, EvSig);
}

void
scene_select_enter(void *context)
{
	TimedRemoteApp *a = context;
	submenu_reset(a->submenu);
	submenu_set_header(a->submenu, "Select Signal");

	sigs = ir_list_alloc();
	if (!sigs)
	{
		submenu_add_item(a->submenu, "(Out of memory)", 0, NULL, NULL);
	} else if (ir_load(a->file, sigs))
	{
		if (sigs->count == 0)
		{
			submenu_add_item(a->submenu, "(No signals in file)", 0, NULL, NULL);
		} else
		{
			for (size_t i = 0; i < sigs->count; i++)
				submenu_add_item(
					a->submenu, furi_string_get_cstr(sigs->items[i].name), i, pick_sig, a);
		}
	} else
	{
		submenu_add_item(a->submenu, "(Error reading file)", 0, NULL, NULL);
	}

	view_dispatcher_switch_to_view(a->vd, ViewMenu);
}

bool
scene_select_event(void *context, SceneManagerEvent event)
{
	TimedRemoteApp *a = context;
	if (event.type != SceneManagerEventTypeCustom || event.event != EvSig)
		return false;
	scene_manager_next_scene(a->sm, ScCfg);
	return true;
}

void
scene_select_exit(void *context)
{
	TimedRemoteApp *a = context;
	submenu_reset(a->submenu);
	if (!sigs)
		return;
	ir_list_free(sigs);
	sigs = NULL;
}
