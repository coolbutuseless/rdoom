//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2008 David Flater
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
//	System interface for sound.
//

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <R.h>  // For Rprintf()

#include "deh_str.h"
#include "i_sound.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomtype.h"


int use_libsamplerate = 0;
float libsamplerate_scale = 0;
boolean use_sfx_prefix = false;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void WriteWAV(char *filename, byte *data, uint32_t length,
                     int samplerate) {
  FILE *wav;
  unsigned int i;
  unsigned short s;

  wav = fopen(filename, "wb");

  // Header

  fwrite("RIFF", 1, 4, wav);
  i = LONG(36 + samplerate);
  fwrite(&i, 4, 1, wav);
  fwrite("WAVE", 1, 4, wav);

  // Subchunk 1

  fwrite("fmt ", 1, 4, wav);
  i = LONG(16);
  fwrite(&i, 4, 1, wav); // Length
  s = SHORT(1);
  fwrite(&s, 2, 1, wav); // Format (PCM)
  s = SHORT(2);
  fwrite(&s, 2, 1, wav); // Channels (2=stereo)
  i = LONG(samplerate);
  fwrite(&i, 4, 1, wav); // Sample rate
  i = LONG(samplerate * 2 * 2);
  fwrite(&i, 4, 1, wav); // Byte rate (samplerate * stereo * 16 bit)
  s = SHORT(2 * 2);
  fwrite(&s, 2, 1, wav); // Block align (stereo * 16 bit)
  s = SHORT(16);
  fwrite(&s, 2, 1, wav); // Bits per sample (16 bit)

  // Data subchunk

  fwrite("data", 1, 4, wav);
  i = LONG(length);
  fwrite(&i, 4, 1, wav);        // Data length
  fwrite(data, 1, length, wav); // Data

  fclose(wav);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void GetSfxLumpName(sfxinfo_t *sfx, char *buf, size_t buf_len) {
  // Linked sfx lumps? Get the lump number for the sound linked to.
  
  if (sfx->link != NULL) {
    sfx = sfx->link;
  }
  
  // Doom adds a DS* prefix to sound lumps; Heretic and Hexen don't
  // do this.
  if (use_sfx_prefix) {
    M_snprintf(buf, buf_len, "ds%s", DEH_String(sfx->name));
  } else {
    M_StringCopy(buf, DEH_String(sfx->name), buf_len);
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_PrecacheSounds(sfxinfo_t *sounds, int num_sounds) {
  char namebuf[9];
  Rprintf("Sound: precache sounds: %i\n", num_sounds);
  for (int i = 0; i < num_sounds; i++) {
    GetSfxLumpName(&sounds[i], namebuf, sizeof(namebuf));
    sounds[i].lumpnum = W_CheckNumForName(namebuf);
    
    if (sounds[i].lumpnum != -1) {
      // CacheSFX(&sounds[i]);
      int lumpnum;
      unsigned int lumplen;
      int samplerate;
      unsigned int length;
      byte *data;
      
      // need to load the sound
      lumpnum = sounds[i].lumpnum;
      data = W_CacheLumpNum(lumpnum, PU_STATIC);
      lumplen = W_LumpLength(lumpnum);
      
      if (lumplen < 8 || data[0] != 0x03 || data[1] != 0x00) {
        // Invalid sound
        Rprintf("----------------------------------- invalid sound\n");
      }
      
      
      samplerate = (data[3] << 8) | data[2];
      length = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];
      
      Rprintf("[% 3i] [%i] %s [%i Hz] %i\n", i, sounds[i].lumpnum,  sounds[i].name, samplerate, length);
      
      // If the header specifies that the length of the sound is greater than
      // the length of the lump itself, this is an invalid sound lump
      
      // We also discard sound lumps that are less than 49 samples long,
      // as this is how DMX behaves - although the actual cut-off length
      // seems to vary slightly depending on the sample rate.  This needs
      // further investigation to better understand the correct
      // behavior.
      if (length > lumplen - 8 || length <= 48) {
        Rprintf("----------------------------------- invalid sound length\n");
      }
      
      
    } else {  
      Rprintf("[% 3i] [%i] %s\n", i, sounds[i].lumpnum,  sounds[i].name);
    }
  }
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Retrieve the raw data lump index
//  for a given SFX name.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int I_RStats_GetSfxLumpNum(sfxinfo_t *sfx) {
  Rprintf("Sound: sfx lump num\n");
  return 1;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_UpdateSoundParams(int handle, int vol, int sep) {
  // Rprintf("Sound: update params\n");
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int I_RStats_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep) {
  Rprintf("Sound: start: ch:%i [% 3i] %s\n", channel, vol, sfxinfo->name);
  // Return -1 for failure
  return channel;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_StopSound(int handle) {
  // Rprintf("Sound: stop\n");
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Check if sound is playing
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static boolean I_RStats_SoundIsPlaying(int handle) {
  // Rprintf("Sound: is playing\n");
  return false;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Periodically called to update the sound system
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_UpdateSound(void) {
  // Rprintf("Sound: update\n");
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void I_RStats_ShutdownSound(void) {
  // Rprintf("Sound: shutdown\n");
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static boolean I_RStats_InitSound(boolean _use_sfx_prefix) {
  
  use_sfx_prefix = _use_sfx_prefix;
  
  Rprintf("Sound: init\n");
  return true;
}

static snddevice_t sound_sdl_devices[] = {
    SNDDEVICE_SB,          
    SNDDEVICE_PAS,         
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER, 
    SNDDEVICE_SOUNDCANVAS, 
    SNDDEVICE_AWE32,
};

sound_module_t DG_sound_module = {
    sound_sdl_devices,       
    arrlen(sound_sdl_devices), 
    I_RStats_InitSound,
    I_RStats_ShutdownSound,     
    I_RStats_GetSfxLumpNum,       
    I_RStats_UpdateSound,
    I_RStats_UpdateSoundParams, 
    I_RStats_StartSound,          
    I_RStats_StopSound,
    I_RStats_SoundIsPlaying,    
    I_RStats_PrecacheSounds,
};
