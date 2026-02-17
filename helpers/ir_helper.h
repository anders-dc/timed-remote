#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <furi.h>
#include <infrared.h>
#include <lib/infrared/signal/infrared_signal.h>

typedef struct
{
	InfraredSignal *signal;
	FuriString *name;
} IrSignalItem;

typedef struct
{
	IrSignalItem *items;
	size_t count;
	size_t capacity;
} IrSignalList;

IrSignalList *ir_list_alloc(void);
void ir_list_free(IrSignalList *list);
bool ir_load(const char *path, IrSignalList *list);
void ir_tx(InfraredSignal *signal);
bool ir_files(const char *dir_path, FuriString ***files, size_t *count);
void ir_files_free(FuriString **files, size_t count);
