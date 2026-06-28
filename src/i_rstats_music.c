//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	System interface for music.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "doomtype.h"
#include "memio.h"
#include "mus2mid.h"

#include "deh_str.h"
#include "gusconf.h"
#include "i_sound.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "sha1.h"
#include "w_wad.h"
#include "z_zone.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_SDL_ShutdownMusic(void) {

}


// Initialize music subsystem
static boolean I_SDL_InitMusic(void) {
  return false;
}



// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
static void UpdateMusicVolume(void) {

}



// Set music volume (0 - 127)
static void I_SDL_SetMusicVolume(int volume) {

}



// Start playing a mid
static void I_SDL_PlaySong(void *handle, boolean looping) {
  
}



static void I_SDL_PauseSong(void) {

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_SDL_ResumeSong(void) {
  
}




static void I_SDL_StopSong(void) {

}



static void I_SDL_UnRegisterSong(void *handle) {

  
}




static void *I_SDL_RegisterSong(void *data, int len) {
  return NULL;
}


// Is the song playing?
static boolean I_SDL_MusicIsPlaying(void) {

  return false;
}



// Poll music position; if we have passed the loop point end position
// then we need to go back.
static void I_SDL_PollMusic(void) {

}


static snddevice_t music_sdl_devices[] = {
    SNDDEVICE_PAS,         SNDDEVICE_GUS,     SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS, SNDDEVICE_GENMIDI, SNDDEVICE_AWE32,
};

music_module_t DG_music_module = {
    music_sdl_devices,    
    arrlen(music_sdl_devices),
    I_SDL_InitMusic,      
    I_SDL_ShutdownMusic,
    I_SDL_SetMusicVolume, 
    I_SDL_PauseSong,
    I_SDL_ResumeSong,     
    I_SDL_RegisterSong,
    I_SDL_UnRegisterSong, 
    I_SDL_PlaySong,
    I_SDL_StopSong,       
    I_SDL_MusicIsPlaying,
    I_SDL_PollMusic,
};
