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
// MikeFC RStats Version:  2026-07-01
// "InitMusic" just returns 'false', so doomgeneric doesn't try playing 
// music 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_ShutdownMusic(void) {

}


// Initialize music subsystem
static boolean I_RStats_InitMusic(void) {
  return false;
}



// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
static void UpdateMusicVolume(void) {

}



// Set music volume (0 - 127)
static void I_RStats_SetMusicVolume(int volume) {

}



// Start playing a mid
static void I_RStats_PlaySong(void *handle, boolean looping) {
  
}



static void I_RStats_PauseSong(void) {

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_ResumeSong(void) {
  
}




static void I_RStats_StopSong(void) {

}



static void I_RStats_UnRegisterSong(void *handle) {

  
}




static void *I_RStats_RegisterSong(void *data, int len) {
  return NULL;
}


// Is the song playing?
static boolean I_RStats_MusicIsPlaying(void) {
  return false;
}



// Poll music position; if we have passed the loop point end position
// then we need to go back.
static void I_RStats_PollMusic(void) {

}


static snddevice_t music_RStats_devices[] = {
    SNDDEVICE_PAS,         SNDDEVICE_GUS,     SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS, SNDDEVICE_GENMIDI, SNDDEVICE_AWE32,
};

music_module_t DG_music_module = {
    music_RStats_devices,    
    arrlen(music_RStats_devices),
    I_RStats_InitMusic,      
    I_RStats_ShutdownMusic,
    I_RStats_SetMusicVolume, 
    I_RStats_PauseSong,
    I_RStats_ResumeSong,     
    I_RStats_RegisterSong,
    I_RStats_UnRegisterSong, 
    I_RStats_PlaySong,
    I_RStats_StopSong,       
    I_RStats_MusicIsPlaying,
    I_RStats_PollMusic,
};
