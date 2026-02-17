#include "ir_helper.h"

#include <flipper_format/flipper_format.h>
#include <furi.h>
#include <infrared/worker/infrared_worker.h>
#include <lib/infrared/signal/infrared_error_code.h>
#include <storage/storage.h>

IrSignalList *
ir_list_alloc(void)
{
	IrSignalList *list = malloc(sizeof(IrSignalList));
	if (!list)
		return NULL;
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
	return list;
}

void
ir_list_free(IrSignalList *list)
{
	if (!list)
		return;

	for (size_t i = 0; i < list->count; i++)
	{
		if (list->items[i].signal)
		{
			infrared_signal_free(list->items[i].signal);
		}
		if (list->items[i].name)
		{
			furi_string_free(list->items[i].name);
		}
	}
	if (list->items)
	{
		free(list->items);
	}
	free(list);
}

static bool
add_sig(IrSignalList *list, InfraredSignal *signal, const char *name)
{
	if (list->count >= list->capacity)
	{
		size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
		IrSignalItem *items = realloc(list->items, new_capacity * sizeof(IrSignalItem));
		if (!items)
			return false;
		list->items = items;
		list->capacity = new_capacity;
	}

	list->items[list->count].signal = signal;
	list->items[list->count].name = furi_string_alloc_set(name);
	if (!list->items[list->count].name)
		return false;
	list->count++;
	return true;
}

bool
ir_load(const char *path, IrSignalList *list)
{
	if (!path || !list)
		return false;

	Storage *storage = furi_record_open(RECORD_STORAGE);
	if (!storage)
		return false;

	FlipperFormat *ff = flipper_format_file_alloc(storage);
	if (!ff)
	{
		furi_record_close(RECORD_STORAGE);
		return false;
	}

	bool success = false;
	FuriString *filetype = NULL;
	FuriString *sig = NULL;

	do
	{
		if (!flipper_format_file_open_existing(ff, path))
			break;

		filetype = furi_string_alloc();
		if (!filetype)
			break;

		uint32_t version;
		bool header_ok = flipper_format_read_header(ff, filetype, &version);
		if (!header_ok)
			break;

		sig = furi_string_alloc();
		if (!sig)
			break;

		while (true)
		{
			InfraredSignal *signal = infrared_signal_alloc();
			if (!signal)
				break;

			InfraredErrorCode err = infrared_signal_read(signal, ff, sig);
			if (err == InfraredErrorCodeNone)
			{
				if (!add_sig(list, signal, furi_string_get_cstr(sig)))
				{
					infrared_signal_free(signal);
					break;
				}
			} else
			{
				infrared_signal_free(signal);
				break;
			}
		}
		success = true;
	} while (false);

	if (filetype)
		furi_string_free(filetype);
	if (sig)
		furi_string_free(sig);
	flipper_format_free(ff);
	furi_record_close(RECORD_STORAGE);
	return success;
}

void
ir_tx(InfraredSignal *signal)
{
	infrared_signal_transmit(signal);
}

static bool
is_ir(const FileInfo *file_info, const char *name)
{
	if ((file_info->flags & FSF_DIRECTORY) || !name)
		return false;

	size_t len = strlen(name);
	return len > 3 && strcmp(name + len - 3, ".ir") == 0;
}

bool
ir_files(const char *dir_path, FuriString ***files,
		     size_t *count)
{
	if (!dir_path || !files || !count)
		return false;

	*files = NULL;
	*count = 0;

	Storage *storage = furi_record_open(RECORD_STORAGE);
	if (!storage)
		return false;

	File *dir = storage_file_alloc(storage);
	if (!dir)
	{
		furi_record_close(RECORD_STORAGE);
		return false;
	}

	if (!storage_dir_open(dir, dir_path))
	{
		storage_file_free(dir);
		furi_record_close(RECORD_STORAGE);
		return false;
	}

	FileInfo file_info;
	char name_buf[256];
	size_t capacity = 0;

	while (storage_dir_read(dir, &file_info, name_buf, sizeof(name_buf)))
	{
		if (!is_ir(&file_info, name_buf))
			continue;

		if (*count >= capacity)
		{
			size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
			FuriString **next_files = realloc(*files, new_capacity * sizeof(FuriString *));
			if (!next_files)
			{
				ir_files_free(*files, *count);
				*files = NULL;
				*count = 0;
				storage_dir_close(dir);
				storage_file_free(dir);
				furi_record_close(RECORD_STORAGE);
				return false;
			}
			*files = next_files;
			capacity = new_capacity;
		}

		(*files)[*count] = furi_string_alloc_set(name_buf);
		if (!(*files)[*count])
		{
			ir_files_free(*files, *count);
			*files = NULL;
			*count = 0;
			storage_dir_close(dir);
			storage_file_free(dir);
			furi_record_close(RECORD_STORAGE);
			return false;
		}
		(*count)++;
	}

	storage_dir_close(dir);
	storage_file_free(dir);
	furi_record_close(RECORD_STORAGE);
	return true;
}

void
ir_files_free(FuriString **files, size_t count)
{
	if (!files)
		return;

	for (size_t i = 0; i < count; i++)
	{
		if (files[i])
		{
			furi_string_free(files[i]);
		}
	}
	free(files);
}
