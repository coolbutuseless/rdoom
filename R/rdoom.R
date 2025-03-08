

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
  
  last_dev <- grDevices::dev.list() |> 
    names() |> 
    tail(1)
  
  if (is.null(last_dev) || last_dev == 'RStudioGD' || endsWith(last_dev, "off_screen")) {
    warning("Slow gfx device detected. Try starting an x11() or windows() device prior to running doom()")
  }
  flush.console()
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Callback for frame drawing
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  draw_frame <- function(nr) {
    grDevices::dev.hold()
    grid::grid.raster(nr, interpolate = FALSE)
    grDevices::dev.flush()
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
    .Call(doom_, wad_file, nframes, draw_frame)
  )
}



if (FALSE) {
  
  x11(type = 'dbcairo', width = 6, height = 4)
  dev.control('inhibit')
  doom(nframes = 1000)

}
