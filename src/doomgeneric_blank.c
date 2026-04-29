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
int frame_num = 0;
bool done = false;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Run doom
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP doom_(SEXP wad_file_, SEXP nframes_, SEXP draw_frame_, SEXP getkey_fun_) {
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
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Fake some command line arguments
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  const char *wad_file = CHAR(STRING_ELT(wad_file_, 0));
  int argc = 3;
  // char *argv[3] = {"doomgeneric", "-iwad", "/Users/mike/projectsdata/doom/wad/doom1.wad"};
  char **argv = calloc(3, sizeof(char *));
  if (argv == NULL) Rf_error("argv failed");
  argv[0] = calloc(1024, sizeof(char));
  argv[1] = calloc(1024, sizeof(char));
  argv[2] = calloc(1024, sizeof(char));
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
  
  frame_num++;
  
  if (frame_num % 100 == 0) {
    Rprintf("Frame: % 6i\n", frame_num);
  }
  
  // Swizzle pixels from BGR to RGBA
  uint8_t *dst = (uint8_t *)canvas;
  uint8_t *src = (uint8_t *)DG_ScreenBuffer;
  for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4; i+=4) {
    dst[i + 0] = src[i + 2];
    dst[i + 1] = src[i + 1];
    dst[i + 2] = src[i + 0];
    dst[i + 3] = 0xFF;
  }
  
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
// @param pressed 1 if key pressed. 0 if key released
// @param doomKey the specific code for the key. See doomgeneric.h 'KEY_*'
// @return 1 if key event happened. Otherwise 0
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int DG_GetKey(int* pressed, unsigned char* doomKey) {
  
  if (done) return 0;
  
  static int toggle = 0;
  
  if (toggle == 1) {
    toggle = 0;
    return 0;
  }
  
  if (!Rf_isNull(getkey_fun)) {
    SEXP getkey_callback = PROTECT(Rf_lang1(getkey_fun)); 
    SEXP res_ = PROTECT(Rf_eval(getkey_callback, R_GlobalEnv));
    int res = Rf_asInteger(res_);
    if (res > 0) {
      toggle = 1;
      *pressed = 1;
      *doomKey = Rf_asInteger(res_) & 0xFF;
      // if (*doomKey != 0) {
      //   Rprintf("%2x ", *doomKey);
      // }
      UNPROTECT(2);
      if (res == KEY_ESCAPE) {
        done = true;
      };
      return 1;
    } else {
      return 0;
    }
  }
  return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: what title should be set for the window?
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DG_SetWindowTitle(const char * title) {
  Rprintf("Title: %s\n", title);
}



