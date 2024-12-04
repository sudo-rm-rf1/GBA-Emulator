#include⠀"gba.h"
#include⠀"cpu.h"
#include⠀"gpu.h"
#include⠀"interrupts.h"
#include⠀"keys.h"
#include⠀"mmu.h"
#include⠀"loader.h"
#include⠀"logger.h"
#include⠀"timer.h"
#include⠀"save.h"
#include⠀"sync.h"

#include⠀<stdio.h>
#include⠀<stdlib.h>

#ifdef⠀WIN32
#include⠀<windows.h>
#else
#include⠀<libgen.h>
#endif

bool⠀init(gba⠀*gba,⠀const⠀char*⠀rom,⠀const⠀char*⠀save_path,⠀const⠀char*⠀bootloader)
{
⠀⠀gba->rom⠀=⠀NULL;
⠀⠀gba->bootloader⠀=⠀NULL;

⠀⠀gba->keys_cumulative_ticks⠀=⠀0;
⠀⠀gba->keys_ticks⠀=⠀15000;

⠀⠀gba->save_timer⠀=⠀0;
⠀⠀gba->save_game_file⠀=⠀NULL;
⠀⠀gba->save_supported⠀=⠀false;

⠀⠀uint32_t⠀rom_size⠀=⠀0;
⠀⠀gba->rom⠀=⠀read_file(rom,⠀&rom_size,⠀true);

⠀⠀if⠀(!gba->rom)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀return⠀false;
⠀⠀⠀⠀}

⠀⠀gb_log_print_rom_info(gba->rom);

⠀⠀cpu_reset(&gba->cpu_reg);

⠀⠀if⠀(!gpu_init(&gba->g,⠀gba->gpu_init_cb))
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀return⠀false;
⠀⠀⠀⠀}

⠀⠀mmu_reset(&gba->mem);

⠀⠀const⠀mbc⠀type⠀=⠀get_type(gba->rom);

⠀⠀if⠀(type⠀==⠀NOT_SUPPORTED)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀return⠀false;
⠀⠀⠀⠀}

⠀⠀mmu_set_rom(&gba->mem,⠀gba->rom,⠀type,⠀rom_size);

#ifdef⠀CGB
⠀⠀switch⠀(gba->rom[0x0143])
⠀⠀{
⠀⠀case⠀0x80:
⠀⠀case⠀0xC0:
⠀⠀⠀⠀gba->mem.cgb_mode⠀=⠀true;
⠀⠀⠀⠀break;
⠀⠀}
#endif

⠀⠀gba->bootloader⠀=⠀read_file(bootloader,⠀NULL,⠀false);

⠀⠀if⠀(gba->bootloader)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀gba->cpu_reg.pc⠀=⠀0x0000;
⠀⠀⠀⠀⠀⠀mmu_set_bootloader(&gba->mem,⠀gba->bootloader);
⠀⠀⠀⠀}
⠀⠀else⠀if⠀(bootloader)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀gb_log⠀(INFO,⠀"No⠀bootloader⠀found⠀at⠀./%s",⠀bootloader);
⠀⠀⠀⠀}

⠀⠀mmu_set_keys(&gba->mem,⠀&gba->k);
⠀⠀keys_reset(&gba->k);

⠀⠀sync_init(&gba->s,⠀100000,⠀gba->ticks_cb);

⠀⠀gba->save_supported⠀=⠀(gba->mem.rom.type⠀&⠀MBC_BATTERY_MASK);

⠀⠀if⠀(gba->save_supported)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀char*⠀base_name_cpy⠀=⠀strdup(rom);
#ifdef⠀WIN32
⠀⠀⠀⠀⠀⠀char⠀base_name[128];
⠀⠀⠀⠀⠀⠀_splitpath(base_name_cpy,⠀NULL,⠀NULL,⠀base_name,⠀NULL);
#else
⠀⠀⠀⠀⠀⠀char*⠀base_name⠀=⠀basename(base_name_cpy);
#endif

⠀⠀⠀⠀⠀⠀const⠀char⠀extension[]⠀=⠀".sav";
⠀⠀⠀⠀⠀⠀const⠀size_t⠀save_file_path_length⠀=⠀strlen(base_name)⠀+⠀strlen(extension)⠀+⠀1⠀+
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀(save_path⠀?⠀strlen(save_path)⠀:⠀0);
⠀⠀⠀⠀⠀⠀gba->save_game_file⠀=⠀malloc(save_file_path_length);
⠀⠀⠀⠀⠀⠀if⠀(save_path)
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀strcpy(gba->save_game_file,⠀save_path);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀strcat(gba->save_game_file,⠀base_name);
⠀⠀⠀⠀⠀⠀⠀⠀}
⠀⠀⠀⠀⠀⠀else
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀strcpy(gba->save_game_file,⠀base_name);
⠀⠀⠀⠀⠀⠀⠀⠀}
⠀⠀⠀⠀⠀⠀strcat(gba->save_game_file,⠀extension);

⠀⠀⠀⠀⠀⠀load_game(gba->save_game_file,⠀&gba->mem);
⠀⠀⠀⠀⠀⠀free(base_name_cpy);
⠀⠀⠀⠀}

⠀⠀return⠀true;
}

void⠀register_keys_callback(gba⠀*gba,⠀keys_cb⠀cb)
{
⠀⠀gba->k_cb⠀=⠀cb;
}

void⠀register_get_ticks_callback(gba⠀*gba,⠀get_ticks_cb⠀cb)
{
⠀⠀gba->ticks_cb⠀=⠀cb;
}

void⠀register_delay_callback(gba⠀*gba,⠀delay_cb⠀cb)
{
⠀⠀gba->delay_cb⠀=⠀cb;
}

void⠀register_gpu_init_callback(gba⠀*gba,⠀gpu_init_cb⠀cb)
{
⠀⠀gba->gpu_init_cb⠀=⠀cb;
}

void⠀register_gpu_uninit_callback(gba⠀*gba,⠀gpu_uninit_cb⠀cb)
{
⠀⠀gba->gpu_uninit_cb⠀=⠀cb;
}

void⠀register_gpu_alloc_image_buffer_callback(gba⠀*gba,⠀gpu_alloc_image_buffer_cb⠀cb)
{
⠀⠀gba->gpu_alloc_image_buffer_cb⠀=⠀cb;
}

void⠀register_gpu_render_callback(gba⠀*gba,⠀gpu_render_cb⠀cb)
{
⠀⠀gba->gpu_render_cb⠀=⠀cb;
}

void⠀register_serial_callback(gba⠀*gba,⠀serial_cb⠀cb)
{
⠀⠀gba->mem.serial_cb⠀=⠀cb;
}

void⠀uninit(gba⠀*gba)
{
⠀⠀save_if_needed(gba);

⠀⠀if⠀(gba->bootloader)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀free(gba->bootloader);
⠀⠀⠀⠀⠀⠀gba->bootloader⠀=⠀NULL;
⠀⠀⠀⠀}

⠀⠀if⠀(gba->rom)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀free(gba->rom);
⠀⠀⠀⠀⠀⠀gba->rom⠀=⠀NULL;
⠀⠀⠀⠀}

⠀⠀gba->gpu_uninit_cb(&gba->g);

⠀⠀gb_log_close_file();
}

void⠀save_if_needed(gba⠀*gba)
{
⠀⠀⠀⠀if⠀(gba->save_supported⠀&&⠀gba->mem.banks.ram.written)
⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀gba->mem.banks.ram.written⠀=⠀false;

⠀⠀⠀⠀⠀⠀⠀⠀save_game(gba->save_game_file,⠀&gba->mem);
⠀⠀⠀⠀⠀⠀}
}

#if⠀CGB
bool⠀get_color_correction(gba⠀*gba)
{
⠀⠀return⠀gba->g.color_correction;
}

void⠀set_color_correction(gba⠀*gba,⠀bool⠀color_correction)
{
⠀⠀gba->g.color_correction⠀=⠀color_correction;
}
#endif

int⠀run(gba⠀*gba)
{
⠀⠀int⠀run_cycles⠀=⠀4194304⠀/⠀4;
⠀⠀while(run_cycles⠀>⠀0)
⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀if⠀(gba->bootloader⠀&&⠀!gba->mem.bootloader_running)
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀mmu_set_bootloader(&gba->mem,⠀NULL);

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀free(gba->bootloader);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->bootloader⠀=⠀NULL;

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀cpu_reset(&gba->cpu_reg);
⠀⠀⠀⠀⠀⠀⠀⠀}

⠀⠀⠀⠀⠀⠀cpu_debug_print(&gba->cpu_reg,⠀ALL);
⠀⠀⠀⠀⠀⠀mmu_debug_print(&gba->mem,⠀ALL);
⠀⠀⠀⠀⠀⠀gpu_debug_print(&gba->g,⠀ALL);

⠀⠀⠀⠀⠀⠀if⠀(cpu_next_command(&gba->cpu_reg,⠀&gba->mem))
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gb_log⠀(ERROR,⠀"Could⠀not⠀process⠀any⠀longer");
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀cpu_debug_print(&gba->cpu_reg,⠀ERROR);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀mmu_debug_print(&gba->mem,⠀ERROR);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀return⠀-1;
⠀⠀⠀⠀⠀⠀⠀⠀}

⠀⠀⠀⠀⠀⠀//⠀STOP⠀should⠀be⠀handled
⠀⠀⠀⠀⠀⠀if⠀(gpu_update(&gba->g,⠀&gba->mem,⠀gba->cpu_reg.clock.last.t,⠀gba->gpu_render_cb,⠀gba->gpu_alloc_image_buffer_cb))
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gb_log⠀(ERROR,⠀"GPU⠀error");
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gpu_debug_print(&gba->g,⠀ERROR);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀return⠀-2;
⠀⠀⠀⠀⠀⠀⠀⠀}

⠀⠀⠀⠀⠀⠀if⠀(!gba->cpu_reg.stop)
⠀⠀⠀⠀⠀⠀⠀⠀timer_update(&gba->cpu_reg,⠀&gba->mem);

⠀⠀⠀⠀⠀⠀if⠀(gba->keys_cumulative_ticks⠀>⠀gba->keys_ticks)
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀if⠀(gba->save_supported⠀&&⠀gba->save_timer++⠀>=⠀10000)
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->save_timer⠀=⠀0;

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀if⠀(gba->mem.banks.ram.written)
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->mem.banks.ram.written⠀=⠀false;

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀save_game(gba->save_game_file,⠀&gba->mem);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀}
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀}

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->keys_cumulative_ticks⠀=⠀0;

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀switch(gba->k_cb(&gba->k))
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀case⠀-1:
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀return⠀1;
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀case⠀1:
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀isr_set_if_flag(&gba->mem,⠀MEM_IF_PIN_FLAG);
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->cpu_reg.halt⠀=⠀⠀false;
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->cpu_reg.stop⠀=⠀⠀false;
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀break;
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀default:
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀break;
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀}
⠀⠀⠀⠀⠀⠀⠀⠀}
⠀⠀⠀⠀⠀⠀else
⠀⠀⠀⠀⠀⠀⠀⠀{
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀gba->keys_cumulative_ticks⠀+=⠀gba->cpu_reg.clock.last.t;
⠀⠀⠀⠀⠀⠀⠀⠀}

⠀⠀⠀⠀⠀⠀sync_time(&gba->s,⠀gba->cpu_reg.clock.last.t,⠀gba->ticks_cb,⠀gba->delay_cb);

⠀⠀⠀⠀⠀⠀run_cycles⠀-=⠀gba->cpu_reg.clock.last.t;
⠀⠀⠀⠀}

⠀⠀return⠀0;
}
