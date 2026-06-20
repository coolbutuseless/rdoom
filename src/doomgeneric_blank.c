#include "doomkeys.h"

#include "doomgeneric.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Global Vars used across all functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t *canvas = NULL;
SEXP draw_callback = NULL;
SEXP getkey_fun = NULL;
SEXP get_mouse_delta_fun = NULL;
int frame_num = 0;
bool done = false;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Run doom
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP doom_(SEXP wad_file_, SEXP draw_frame_, SEXP getkey_fun_, SEXP get_mouse_delta_fun_) {
  int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a native raster canvas to copy the drawing buffer into
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP nr_ = PROTECT(Rf_allocMatrix(INTSXP, DOOMGENERIC_RESY, DOOMGENERIC_RESX)); nprotect++;
  SEXP class_ = PROTECT(Rf_mkString("nativeRaster")); nprotect++;
  SET_CLASS(nr_, class_);
  canvas = (uint32_t *)INTEGER(nr_);
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Prepare the function call which will be called every frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  draw_callback   = PROTECT(Rf_lang2(draw_frame_, nr_)); nprotect++;
  getkey_fun = getkey_fun_;
  get_mouse_delta_fun = get_mouse_delta_fun_;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Fake some command line arguments
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  const char *wad_file = CHAR(STRING_ELT(wad_file_, 0));
  int argc = 3;
  // char *argv[3] = {"doomgeneric", "-iwad", "/Users/mike/projectsdata/doom/wad/doom1.wad"};
  char **argv = (char **)R_alloc(sizeof(char *), 3);
  if (argv == NULL) Rf_error("argv failed");
  argv[0] = R_alloc(sizeof(char), 1024); memset(argv[0], 0, sizeof(char) * 1000);
  argv[1] = R_alloc(sizeof(char), 1024); memset(argv[1], 0, sizeof(char) * 1000);
  argv[2] = R_alloc(sizeof(char), 1024); memset(argv[2], 0, sizeof(char) * 1000);
  strncpy(argv[0], "doom", 4);
  strncpy(argv[1], "-iwad", 5);
  strncpy(argv[2], wad_file, strlen(wad_file));
  
  
  doomgeneric_Create(argc, argv);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Run Time Steps (ticks) in Doom
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  done = false;
  while (!done) {
    doomgeneric_Tick();
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
  canvas = NULL;
  UNPROTECT(nprotect);
  return nr_;
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: doing initialisation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DG_Init(void) {
  Rprintf("DG_Init: (%i, %i)\n", DOOMGENERIC_RESX, DOOMGENERIC_RESY);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: Frame is ready
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DG_DrawFrame(void) {
  
  // Swizzle pixels from BGR to RGBA
  uint8_t *dst = (uint8_t *)canvas;
  uint8_t *src = (uint8_t *)DG_ScreenBuffer;
  for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4; i+=4) {
    dst[i + 0] = src[i + 2];
    dst[i + 1] = src[i + 1];
    dst[i + 2] = src[i + 0];
    dst[i + 3] = 0xFF;
  }
  
  if (!done)
    Rf_eval(draw_callback, R_GlobalEnv);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: Sleep for a bit
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DG_SleepMs(uint32_t ms) {
    usleep (ms * 1000);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: time in ms
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t DG_GetTicksMs(void) {
    struct timeval  tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);

    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: keys pressed
//
// The engine tick calls this function over-and-over until it returns 0.
// This means that the R callback needs to take things out of the current key
// state, handle them, and nullify that key before the next call to the callback
//
// @param pressed 1 if key pressed. 0 if key released
// @param doomKey the specific code for the key. See doomgeneric.h 'KEY_*'
//
// @return 1 if key event happened. Otherwise 0
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int DG_GetKey(int* pressed, unsigned char* doomKey) {
  
  // If this callback gets called 20 times in a single frame, it means that
  // there's a bug in the key input handling.
  // In theory, the tigr key handler only holds a maximum of 6 keys.
  // Any more than 6 calls is an indication that I've mucked up the
  // key handling
  static int count = 0;
  if (count > 20) {
    Rprintf("Possible key input loop\n");
    done = true;
  }
  
  // If game is 'done' for whatever reason, don't bother processing keys
  if (done) return 0;

  
  if (!Rf_isNull(getkey_fun)) {
    SEXP getkey_callback = PROTECT(Rf_lang1(getkey_fun));
    SEXP res_ = PROTECT(Rf_eval(getkey_callback, R_GlobalEnv));
    int *res = INTEGER(res_);
    if (res[0] == -1) {
      count = 0;
      UNPROTECT(2);
      return 0;
    }
    
    *pressed = res[0];        // 0 or 1 for 'released' or 'pressed'
    *doomKey = res[1] & 0xFF; // Convert to unsigned char
    
    // Rprintf("= %i %x\n", *pressed, *doomKey);
    count++;
    UNPROTECT(2);
    if (*doomKey == KEY_ESCAPE) done = true;
    return 1;
  }
  
  count = 0;
  return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: what title should be set for the window?
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DG_SetWindowTitle(const char * title) {
  Rprintf("Title: %s\n", title);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Mouse movement
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int32_t DG_GetMouseDelta(void) {
  
  int32_t res = 1;
  
  if (!Rf_isNull(get_mouse_delta_fun)) {
    SEXP get_mouse_delta_callback = PROTECT(Rf_lang1(get_mouse_delta_fun));
    SEXP res_ = PROTECT(Rf_eval(get_mouse_delta_callback, R_GlobalEnv));
    res = INTEGER(res_)[0];
  }

  
  return res;
}

