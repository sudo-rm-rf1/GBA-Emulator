#ifndef⠀gba_H
#define⠀gba_H

#include⠀"gba_internal.h"

void⠀register_keys_callback(gba⠀*gba,⠀keys_cb⠀cb);

void⠀register_get_ticks_callback(gba⠀*gba,⠀get_ticks_cb⠀cb);

void⠀register_delay_callback(gba⠀*gba,⠀delay_cb⠀cb);

void⠀register_gpu_init_callback(gba⠀*gba,⠀gpu_init_cb⠀cb);

void⠀register_gpu_uninit_callback(gba⠀*gba,⠀gpu_uninit_cb⠀cb);

void⠀register_gpu_alloc_image_buffer_callback(gba⠀*gba,⠀gpu_alloc_image_buffer_cb⠀cb);

void⠀register_gpu_render_callback(gba⠀*gba,⠀gpu_render_cb⠀cb);

void⠀register_serial_callback(gba⠀*gba,⠀serial_cb⠀cb);

void⠀save_if_needed(gba⠀*gba);

#if⠀CGB
bool⠀get_color_correction(gba⠀*gba);

void⠀set_color_correction(gba⠀*gba,⠀bool⠀color_correction);
#endif

bool⠀init(gba⠀*gba,⠀const⠀char*⠀rom,⠀const⠀char*⠀save_path,⠀const⠀char*⠀bootloader);

void⠀uninit(gba⠀*gba);

int⠀run(gba⠀*gba);

#endif
