
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP doom_(SEXP wad_file_, SEXP nframes_, SEXP draw_frame_, SEXP getkey_fun_);
static const R_CallMethodDef CEntries[] = {
  
  {"doom_", (DL_FUNC) &doom_, 4},
  {NULL , NULL, 0}
};


void R_init_rdoom(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}



