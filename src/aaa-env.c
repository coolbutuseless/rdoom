
#define R_NO_REMAP

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "aaa-env.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get the variable 'varname' from the given environment
//
// Note: the variable *MUST* exist in the environment, otherwise an error
//       will be thrown!
//
// @param varname character name
// @param env environment
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP get_var(SEXP env_, const char *varname) {

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // check that i'm not an idiot
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (TYPEOF(env_) != ENVSXP) {
    Rf_error("get_var(): 'env' argument is not an ENVSCP");
  }


  SEXP nm1_ = PROTECT(Rf_mkChar(varname));
  SEXP nm_  = PROTECT(Rf_installChar(nm1_));
  SEXP rval_ = R_getVar(nm_, env_, TRUE);


  UNPROTECT(2);
  return rval_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Set a variable in the environment
//
// @param varname charaacter
// @param value to set
// @param env environment
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void set_var(SEXP env_, const char *varname, SEXP value_) {

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check that I'm not an idiot and actually have an environment object
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (TYPEOF(env_) != ENVSXP) {
    Rf_error("set_var(): 'env' argument is not an ENVSCP");
  }
  
  
  SEXP nm1_ = PROTECT(Rf_mkChar(varname));
  SEXP nm_  = PROTECT(Rf_installChar(nm1_));
  
  Rf_defineVar(nm_, value_, env_);

  UNPROTECT(2);
}
