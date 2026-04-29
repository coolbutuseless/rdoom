

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Run doom in demo mode. No keyboard input.
#' 
#' @param nframes number of frames to run. Note the first 300 frames are a 
#'        a static image of the Doom logo.
#' @param wad_file full path to WAD file. Default: use the demo 'doom1.wad' 
#'        included with this package.
#' @return None
#' @import grid
#' @import grDevices
#' @importFrom utils tail flush.console
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
doom <- function(nframes = 100, wad_file = system.file("doom1.wad", package = "rdoom", mustWork = TRUE)) {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Callback for frame drawing. Two choices
  #  - naratigr
  #  - grid.raster (try and open a fast graphics device before running doom())
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (requireNamespace('naratigr', quietly = TRUE)) {
   
    window <- naratigr::tigr_open(width = 640, height = 400, title = "RDoom", expand = 2)
    
    draw_frame <- function(nr) {
      naratigr::tigr_update(window, nr)
    }
    get_key <- function() {
      state <- naratigr::tigr_state(window)
      
      if ('x' %in% state$keys$char$held) {
        doom_keys$x
      } else if ('RETURN' %in% state$keys$special$held) {
        doom_keys$RETURN
      } else if ("LEFT" %in% state$keys$special$held) {
        doom_keys$LEFT
      } else if ("RIGHT" %in% state$keys$special$held) {
        doom_keys$RIGHT
      } else if ("UP" %in% state$keys$special$held) {
        doom_keys$UP
      } else if ("DOWN" %in% state$keys$special$held) {
        doom_keys$DOWN    
      } else if ("ESC" %in% state$keys$special$held) {
        doom_keys$ESC      
      } else {
        0L
      }
        
    }
     
  } else {
    last_dev <- grDevices::dev.list() |> 
      names() |> 
      tail(1)
    
    if (is.null(last_dev) || last_dev == 'RStudioGD' || endsWith(last_dev, "off_screen")) {
      warning("Slow gfx device detected. Try starting an x11() or windows() device prior to running doom()")
    }
    flush.console()
    
    draw_frame <- function(nr) {
      grDevices::dev.hold()
      grid::grid.raster(nr, interpolate = FALSE)
      grDevices::dev.flush()
    }
    
    get_key <- NULL;
    
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
    .Call(doom_, wad_file, nframes, draw_frame, get_key)
  )
  
  message("Finished running doom. Control returning to R.")
  naratigr::tigr_close(window)
}


doom_keys <- list(
  RIGHT  = 0xae,
  LEFT   = 0xac,
  UP     = 0xad,
  DOWN   = 0xaf,
  ESC    =   27,
  RETURN =   13,
  TAB    =    9,
  x      = 0xa3  # fire
) |> lapply(as.integer)


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
  
  x11(type = 'dbcairo', width = 6, height = 4)
  dev.control('inhibit')
  doom(nframes = 1000)

}
