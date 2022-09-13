#ifndef WINDOW_RENDERER_H
#define WINDOW_RENDERER_H

#ifdef OPENCV_ENABLED
#include "OpencvRenderer.h"
#endif

#ifdef SDL_ENABLED
#include "SdlRenderer.h"
#endif

#include "WindowRendererInterface.h"

enum e_renderer_type
{
  RENDERER_TYPE,
#ifdef SDL_ENABLED
  RENDERER_TYPE_SDL,
#endif
#ifdef OPENCV_ENABLED
  RENDERER_TYPE_OPENCV
#endif
};

inline WindowRendererInterface *createOutputRenderer(e_renderer_type renderer_type, const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type = INPUT_BUFFER_TYPE_CPU, int screen = 0, bool fullscreen = false, s_color font_color = s_color(), std::ostream *error = NULL)
{
  WindowRendererInterface *out_interface = NULL;
  switch(renderer_type)
  {
  case RENDERER_TYPE:
	#ifdef SDL_ENABLED
		out_interface = new SdlRenderer(window_name, width, height, dev_type, screen, fullscreen, font_color, error);
		if (out_interface->isOpened()) break;
	#endif
	#ifdef OPENCV_ENABLED
		out_interface = new OpencvRenderer(window_name, width, height, dev_type, screen, fullscreen, font_color, error);
		if (out_interface->isOpened()) break;
	#endif
	  break;
#ifdef SDL_ENABLED
  case RENDERER_TYPE_SDL:
    out_interface = new SdlRenderer(window_name, width, height, dev_type, screen, fullscreen, font_color, error);
	break;
#endif
#ifdef OPENCV_ENABLED
  case RENDERER_TYPE_OPENCV:
    out_interface = new OpencvRenderer(window_name, width, height, dev_type, screen, fullscreen, font_color, error);
    break;
#endif
  }
  if(!out_interface->isOpened())
  {
    delete out_interface;
    if(error != NULL) *error << "Output renderer info", "Output renderer isnt at valid state.";
    return NULL;
  }

  return out_interface;
}

inline WindowRendererInterface *createOutputRenderer(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type = INPUT_BUFFER_TYPE_CPU, int screen = 0, bool fullscreen = false, s_color font_color = s_color(), std::ostream *error = NULL)
{
	return createOutputRenderer(RENDERER_TYPE, window_name, width, height, dev_type, screen, fullscreen, font_color, error);
}

#endif