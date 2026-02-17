#pragma once

#include <stddef.h>
#include <stdint.h>

uint32_t time_hms_sec(uint8_t h, uint8_t m, uint8_t s);
void time_sec_hms(uint32_t total_seconds, uint8_t *h, uint8_t *m, uint8_t *s);

#ifndef TIMED_REMOTE_TEST_BUILD
uint32_t time_until(uint8_t target_h, uint8_t target_m, uint8_t target_s);
void time_name(char *buffer, size_t buffer_size);
#endif
