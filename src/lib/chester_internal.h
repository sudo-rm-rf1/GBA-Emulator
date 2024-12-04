#ifndef⠀gba_INTERNAL_H
#define⠀gba_INTERNAL_H

#include⠀"cpu.h"
#include⠀"gpu.h"
#include⠀"keys.h"
#include⠀"mmu.h"
#include⠀"sync.h"

struct⠀gba_s⠀{
⠀⠀registers⠀cpu_reg;
⠀⠀memory⠀mem;
⠀⠀gpu⠀g;
⠀⠀keys⠀k;
⠀⠀sync_timer⠀s;
⠀⠀uint8_t*⠀bootloader;
⠀⠀uint8_t*⠀rom;
⠀⠀int⠀keys_cumulative_ticks;
⠀⠀int⠀keys_ticks;
⠀⠀unsigned⠀int⠀save_timer;
⠀⠀char*⠀save_game_file;
⠀⠀bool⠀save_supported;

⠀⠀//⠀callbacks
⠀⠀keys_cb⠀k_cb;
⠀⠀get_ticks_cb⠀ticks_cb;
⠀⠀delay_cb⠀delay_cb;
⠀⠀gpu_init_cb⠀gpu_init_cb;
⠀⠀gpu_uninit_cb⠀gpu_uninit_cb;
⠀⠀gpu_alloc_image_buffer_cb⠀gpu_alloc_image_buffer_cb;
⠀⠀gpu_render_cb⠀gpu_render_cb;
};

typedef⠀struct⠀gba_s⠀gba;

#endif
