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
SEXP callback = NULL;
int frame_num = 0;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Run doom
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP doom_(SEXP wad_file_, SEXP nframes_, SEXP draw_frame_) {
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
  callback = PROTECT(Rf_lang2(draw_frame_, nr_)); nprotect++;
  
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
  for (int i = 0; i < Rf_asInteger(nframes_); i++) {
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
    Rprintf(".");
  }
  
  uint8_t *dst = (uint8_t *)canvas;
  uint8_t *src = (uint8_t *)DG_ScreenBuffer;
  for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4; i+=4) {
    dst[i + 0] = src[i + 2];
    dst[i + 1] = src[i + 1];
    dst[i + 2] = src[i + 0];
    dst[i + 3] = 0xFF;
  }
  
  Rf_eval(callback, R_GlobalEnv);
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int DG_GetKey(int* pressed, unsigned char* doomKey) {
  return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Callback: what title should be set for the window?
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DG_SetWindowTitle(const char * title) {
  Rprintf("Title: %s\n", title);
}



