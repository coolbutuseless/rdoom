
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Doom keys which can be set by the user
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
configurable_doom_keys <- c(
  "KEY_RIGHTARROW", 
  "KEY_LEFTARROW",	 
  "KEY_UPARROW",		 
  "KEY_DOWNARROW",	 
  "KEY_STRAFE_L",
  "KEY_STRAFE_R",
  "KEY_FIRE",
  "KEY_USE",		
  "KEY_PAUSE",	    
  "KEY_ESCAPE",	   
  "KEY_ENTER",		   
  "KEY_TAB",				   
  "KEY_BACKSPACE",	 
  "KEY_RSHIFT",	      
  "KEY_LALT",
  "KEY_F1",
  "KEY_F2",
  "KEY_F3",
  "KEY_F4",
  "KEY_F5",
  "KEY_F6",
  "KEY_F7",
  "KEY_F8"
)



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# All single letter keys returned by 'tigerfb' window state
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
alpha_keys <- tigerfb::fb_key_names() |> 
  setdiff(LETTERS) 
alpha_keys <- alpha_keys[nchar(alpha_keys) == 1]


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Create an sanitise a keymap specification
#' 
#' @param ... name/value pairs with the name representing a valid 'tigerfb'
#'        key name (See \code{tigerfb::fb_key_names()}), and the value
#'        representing a doom key name (See \code{configuratble_doom_keys})
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
keymap_create <- function(...) {
  
  ll <- list(...)
  stopifnot("All arguments must be named" = !is.null(names(ll)))
  
  bad <- setdiff(names(ll), tigerfb::fb_key_names())
  if (length(bad) > 0) {
    stop("These 'tigerfb' names do not exist [see tigerfb::fb_key_names() ]: ",
         deparse1(bad))
  }

  bad <- setdiff(unname(unlist(ll)), configurable_doom_keys) 
  if (length(bad) > 0) {
    stop("These 'doom' keys do not exist [see 'configurable_doom_keys': ", 
         deparse1(bad))
  }
  

  ll
}



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Create a default keymap
#' 
#' Note: multiple keys can map to a doom function.  In the default keymap,
#' both SPACE and CTRL map to "KEY_FIRE"
#' 
#' @return named list of key mappings from tigerfb to doom
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
keymap_default <- function() {
  keymap_create(
    RIGHT     = "KEY_RIGHTARROW", 
    LEFT      = "KEY_LEFTARROW",	 
    
    UP        = "KEY_UPARROW",		 
    DOWN      = "KEY_DOWNARROW",	 
    
    w         = "KEY_UPARROW",
    s         = "KEY_DOWNARROW",
    a         = "KEY_STRAFE_L",
    d         = "KEY_STRAFE_R",
    
    ` `       = "KEY_FIRE",
    e         = "KEY_USE",		
    p         = "KEY_PAUSE",	     
    
    CTRL      = "KEY_FIRE",		   
    ESC       = "KEY_ESCAPE",	   
    RETURN    = "KEY_ENTER",		   
    TAB       = "KEY_TAB",			   
    F1        = "KEY_F1",
    F2        = "KEY_F2",
    F3        = "KEY_F3",
    F4        = "KEY_F4",
    F5        = "KEY_F5",
    F6        = "KEY_F6",
    F7        = "KEY_F7",
    F8        = "KEY_F8",   
    BACKSPACE = "KEY_BACKSPACE",	 
    SHIFT     = "KEY_RSHIFT",	     
    ALT       = "KEY_LALT"
  )
}





#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Run doom in demo mode. No keyboard input.
#' 
#' @param wad_file full path to WAD file. Default: use the demo 'doom1.wad' 
#'        included with this package.
#' @param mouse use the mouse? Default: FALSE
#' @param sensitivity 75
#' @param keymap created with 'keymap_default()' or 'keymap_create()'
#' @param ... further options passed to \code{tigerfb::fb_open()}
#' @return None
#' @import grid
#' @import grDevices
#' @importFrom utils tail flush.console
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
doom <- function(wad_file = system.file("doom1.wad", package = "rdoom", mustWork = TRUE), 
                 mouse = FALSE,
                 sensitivity = 150,
                 keymap = keymap_default(),
                 ...) {
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Find the WAD file
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wad_file <- normalizePath(wad_file)
  stopifnot(file.exists(wad_file))
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Do the key map translation from 'tigerfb' to actual doom key code (integer)
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  tigerfb_keys_to_doom_code <- keymap |> 
    lapply(\(x) doom_keys_to_code[[x]])
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Open a window
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  window <- tigerfb::fb_open(width = 640, height = 400, title = "RDoom", ...)
  on.exit({tigerfb::fb_close(window); rm(window); Sys.sleep(1); gc(); Sys.sleep(1);})
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Define the callback for drawing a frame
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  draw_frame <- function(nr) {
    tigerfb::fb_update(window, nr)
  }
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # An environment for keeping track of which key presses have been processed
  #
  # The 'get_key' callback is called multiple-times-per-frame until all
  # key presses in the queue have been processed.
  #
  # Note: We only need to feed doom the key transitions
  #   i.e. from-pressed-to-not-pressed or from-not-pressed-to-pressed
  # The engine assumes that that once a key is pressed, it remains pressed
  # until it gets an explicit "key_released" return value from this function.
  #
  # Need to track the current 'key_pressed' state over multiple calls to 'get_key()'
  # as this callback is called multiple times by the doom engine.  Each time it 
  # is called, 'get_key()' is expected to return either
  #  1)  A doom key code and a pressed/released value (i.e. 1 or 0, respectively)
  #  2)  Return (-1L, -1L) to indicate there are no more key events to handle
  #      for this frame.
  keys       <- new.env()
  keys$down0 <- NULL
  keys$down  <- NULL
  keys$prior <- NULL
  keys$up    <- NULL
  keys$ascii_only <- FALSE
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Define the function which is called possibly manytimes per frame in order to log 
  # keyboard events.
  # This function gets called multiple times until all buffered events
  # are exhausted - and it returns (-1L, -1L) to denote exhaustion.
  # For each key event that is occurring this function returns
  #   - (1, integer-key-value) if the key was pressed
  #   - (0, integer-key-value) if the key was released
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  get_key <- function() {
    
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # 
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (is.null(keys$down)) {
      state <- tigerfb::fb_state(window)
      
      # names of keys which are pressed
      all_keys_now   <- names(state$key)[state$key > 0]
      doom_keys_now  <- intersect(names(tigerfb_keys_to_doom_code), all_keys_now)
      alpha_keys_now <- intersect(all_keys_now, alpha_keys) |> paste(collapse = "") |> utf8ToInt()
      
      if (keys$ascii_only) {
        keys$down0  <- c(
          alpha_keys_now
        )
      } else {
        keys$down0  <- c(
          tigerfb_keys_to_doom_code[doom_keys_now] |> unlist() |> unname(),
          alpha_keys_now
        )
      }
      
      keys$down   <- keys$down0[ !keys$down0 %in% keys$prior ]
      keys$up     <- keys$prior[ !keys$prior %in% keys$down0 ]
      
      keys$prior  <- keys$down0

      # cat(state$screen[[1]], "v0 [", keys$down0, "]    v [", keys$down, "]    ^ [",
          # keys$up, "]    - [", keys$prior, "]\n")
      
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      # When the letter keys are assigned to controls, this interferes with how
      # typing cheat codes works. The workaround is this:
      #   'p' - pause the game
      #   'F1' - turn on ascii-only mode
      #   Type the cheat code
      #   'F2' - turn back on full controls
      #   'p'  - unpause the game
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if ('F9' %in% all_keys_now) {
        keys$ascii_only <- TRUE
      }
      
      if ('F10' %in% all_keys_now) {
        keys$ascii_only <- FALSE
      }
      
      
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      # Do mouse button handling here. 
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (mouse) {
        if (state$mouse$buttons[1]) {
          keys$down <- c(keys$down, doom_keys_to_code[['KEY_FIRE']])
        } else {
          keys$up   <- c(keys$up  , doom_keys_to_code[['KEY_FIRE']])
        }
      }
      
      
    }
    
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # If there are still keys to process, pop one off the stack and
    # return (1, key-value)
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (length(keys$down) > 0L) {
      key <- keys$down[[1]]
      keys$down <- keys$down[-1L]
      # message("v  ", key)
      return(c(1L, key))
    }

    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # If there were keys pressed last time that aren't pressed now,
    # then signal this by returning (0, key-value)
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (length(keys$up) > 0L) {
      key <- keys$up[[1]]
      keys$up <- keys$up[-1L]
      # message("^ ", key)
      return(c(0L, key))
    }
    
    
    
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # If we get here, then
    #   - all doom related keys have been signalled back to the doom engine
    #   - 'key_pressed_prior' holds the key state at the start of this frame
    #     and can be used in future calls to decide if a key has changed from
    #     pressed-to-released or vice versa.
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    keys$down  <- NULL
    keys$up    <- NULL
  
    # Return a custom signal to indicate that there are no more keys to 
    # process for this frame
    return(c(-1L, -1L))
  }
  
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Callback for mouse delta
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  xpos_prior <- NULL
  
  get_mouse_delta <- function() {
    
    if (!isTRUE(mouse)) return(0L)
    
    state <- tigerfb::fb_state(window)
    
    xpos <- state$mouse$coords[[1]]
    
    if (is.null(xpos_prior)) {
      xpos_prior <<- xpos
    }
    
    delta <- (xpos_prior - xpos) * sensitivity
    xpos_prior <<- xpos
    
    
    return(as.integer(delta))
  }
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Callback for image saving
  # magick -delay 1 anim/*.png -delay 1 -geometry 320x200 anim.gif
  # gifsicle -k 64 --lossy=10 --delay 4 -o anim2.gif anim.gif
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # i <- 0L
  # draw_frame <- function(nr) {
  #   i <<- i + 1L
  #   if (i %% 2 == 0) {
  #     filename <- sprintf("working/screens/image-%04i.png", i)
  #     fastpng::write_png(nr, filename, compression_level = 0, use_filter = FALSE)
  #   }
  # }
  
  invisible(
    .Call(doom_, wad_file, draw_frame, get_key, get_mouse_delta)
  )
  
  message("---------------------------------------------------------------")
  message("Finished running doom. Control returning to R.")
  message("---------------------------------------------------------------")
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# These are the reference doom key values from 'doomgeneric.h'
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
doom_keys_to_code <- list(
  KEY_RIGHTARROW = 0xae,
  KEY_LEFTARROW	 = 0xac,
  
  KEY_UPARROW		 = 0xad,
  KEY_DOWNARROW	 = 0xaf,
  KEY_STRAFE_L	 = 0xa0,
  KEY_STRAFE_R	 = 0xa1,
  
  KEY_USE			   = 0xa2,
  KEY_FIRE		   = 0xa3,
  KEY_ESCAPE	   = 27,
  KEY_ENTER		   = 13,
  KEY_TAB			   = 9,
  KEY_F1			   = (0x80+0x3b),
  KEY_F2			   = (0x80+0x3c),
  KEY_F3			   = (0x80+0x3d),
  KEY_F4			   = (0x80+0x3e),
  KEY_F5			   = (0x80+0x3f),
  KEY_F6			   = (0x80+0x40),
  KEY_F7			   = (0x80+0x41),
  KEY_F8			   = (0x80+0x42),
  # KEY_F9			   = (0x80+0x43),
  # KEY_F10			   = (0x80+0x44),
  # KEY_F11			   = (0x80+0x57),
  # KEY_F12			   = (0x80+0x58),
  KEY_BACKSPACE	 = 0x7f,
  KEY_PAUSE	     = 0xff,
  KEY_RSHIFT	   = (0x80+0x36),
  KEY_RCTRL      = (0x80+0x1d),
  KEY_RALT       = (0x80+0x38),
  KEY_LALT       = (0x80+0x38)
) |> lapply(as.integer)


# #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# # Mapping from 'tigr' key names to ref_doom_key values
# #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# tigerfb_keys_to_doom_code <- list(
#   RIGHT     = "KEY_RIGHTARROW", 
#   LEFT      = "KEY_LEFTARROW",	 
#   
#   UP        = "KEY_UPARROW",		 
#   DOWN      = "KEY_DOWNARROW",	 
#   
#   w         = "KEY_UPARROW",
#   s         = "KEY_DOWNARROW",
#   a         = "KEY_STRAFE_L",
#   d         = "KEY_STRAFE_R",
#   
#   ` `       = "KEY_FIRE",
#   e         = "KEY_USE",		
#   p         = "KEY_PAUSE",	     
#   
#   CTRL      = "KEY_FIRE",		   
#   ESC       = "KEY_ESCAPE",	   
#   RETURN    = "KEY_ENTER",		   
#   TAB       = "KEY_TAB",			   
#   F1        = "KEY_F1",
#   F2        = "KEY_F2",
#   F3        = "KEY_F3",
#   F4        = "KEY_F4",
#   F5        = "KEY_F5",
#   F6        = "KEY_F6",
#   F7        = "KEY_F7",
#   F8        = "KEY_F8",
#   # F9        = "KEY_F9",			   
#   # F10       = "KEY_F10",			   
#   # F11       = "KEY_F11",			   
#   # F12       = "KEY_F12",			   
#   BACKSPACE = "KEY_BACKSPACE",	 
#   SHIFT     = "KEY_RSHIFT",	   
#   # CTRL      = "KEY_RCTRL",      
#   # ALT       = "KEY_RALT",       
#   ALT       = "KEY_LALT"
# ) |> lapply(\(x) doom_keys_to_code[[x]])


# #define KEY_RIGHTARROW	0xae
# #define KEY_LEFTARROW	0xac
# #define KEY_UPARROW		0xad
# #define KEY_DOWNARROW	0xaf
# #define KEY_STRAFE_L	0xa0
# #define KEY_STRAFE_R	0xa1
# #define KEY_USE			0xa2
# #define KEY_FIRE		0xa3
# #define KEY_ESCAPE		27
# #define KEY_ENTER		13
# #define KEY_TAB			9
# #define KEY_F1			(0x80+0x3b)
# #define KEY_F2			(0x80+0x3c)
# #define KEY_F3			(0x80+0x3d)
# #define KEY_F4			(0x80+0x3e)
# #define KEY_F5			(0x80+0x3f)
# #define KEY_F6			(0x80+0x40)
# #define KEY_F7			(0x80+0x41)
# #define KEY_F8			(0x80+0x42)
# #define KEY_F9			(0x80+0x43)
# #define KEY_F10			(0x80+0x44)
# #define KEY_F11			(0x80+0x57)
# #define KEY_F12			(0x80+0x58)
# 
# #define KEY_BACKSPACE	0x7f
# #define KEY_PAUSE	0xff
# 
# #define KEY_EQUALS	0x3d
# #define KEY_MINUS	0x2d
# 
# #define KEY_RSHIFT	(0x80+0x36)
# #define KEY_RCTRL	(0x80+0x1d)
# #define KEY_RALT	(0x80+0x38)
# 
# #define KEY_LALT	KEY_RALT
# 
# // new keys:
#   
#   #define KEY_CAPSLOCK    (0x80+0x3a)
#   #define KEY_NUMLOCK     (0x80+0x45)
#   #define KEY_SCRLCK      (0x80+0x46)
#   #define KEY_PRTSCR      (0x80+0x59)
#   
#   #define KEY_HOME        (0x80+0x47)
#   #define KEY_END         (0x80+0x4f)
#   #define KEY_PGUP        (0x80+0x49)
#   #define KEY_PGDN        (0x80+0x51)
#   #define KEY_INS         (0x80+0x52)
#   #define KEY_DEL         (0x80+0x53)
#   
#   #define KEYP_0          0
#   #define KEYP_1          KEY_END
#   #define KEYP_2          KEY_DOWNARROW
#   #define KEYP_3          KEY_PGDN
#   #define KEYP_4          KEY_LEFTARROW
#   #define KEYP_5          '5'
#   #define KEYP_6          KEY_RIGHTARROW
#   #define KEYP_7          KEY_HOME
#   #define KEYP_8          KEY_UPARROW
#   #define KEYP_9          KEY_PGUP
#   
#   #define KEYP_DIVIDE     '/'
#   #define KEYP_PLUS       '+'
#   #define KEYP_MINUS      '-'
#   #define KEYP_MULTIPLY   '*'
#   #define KEYP_PERIOD     0
#   #define KEYP_EQUALS     KEY_EQUALS
#   #define KEYP_ENTER      KEY_ENTER





if (FALSE) {
  
  doom()
  
}
