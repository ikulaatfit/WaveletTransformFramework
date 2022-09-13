#ifndef SDL_RENDERER_H
#define SDL_RENDERER_H
#ifdef SDL_ENABLED
#include <SDL.h>
#include <SDL_ttf.h>
//#include <GL/glew.h>

#include "WindowRendererInterface.h"
#include <atomic>

/**
 * @brief Video getting class
 */
class SdlRenderer : public WindowRendererInterface
{
public:
  /**
   * Initialize object and register ffmpeg codec.
   */
  SdlRenderer();
  /**
   * Initialize object, register ffmpeg codec and open video file.
   * @param v_file video file path
   * @param width width of loaded video or 0 if fail
   * @param height height of loaded video or 0 if fail
   * @param error optional string to write error
   */
  SdlRenderer(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type = INPUT_BUFFER_TYPE_CPU, int screen = 0, bool fullscreen = false, s_color font_color = s_color(), std::ostream *error = NULL);
  /**
   * Close video file if opened.
   */
  ~SdlRenderer();
  /**
   * Return file open state.
   * @return file open state
   */
	bool isOpened();
  /**
   * Open video file.
   * @param v_file video file path
   * @param width width of loaded video or 0 if fail
   * @param height height of loaded video or 0 if fail
   * @param error optional string to write error
   * @return success of open file
   */
  bool open(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type = INPUT_BUFFER_TYPE_CPU, int screen = 0, bool fullscreen = false, s_color font_color = s_color(), std::ostream *error = NULL);
  /**
   * Close video file if opened.
   */
	void close();
  /**
   * Get next frame.
   * @param bgra_frame output frame
   * @param error optional string to write error
   * @return true if return regular frame or false if reach end of file
   */
  bool showFrame(void *bgra_frame, std::string &output_text, unsigned int text_start_x = 10, unsigned int text_start_y = 10, std::ostream *error = NULL);
  unsigned char getKey();
private:
  /**
   * Initialize object and register ffmpeg codec.
   */
  void init();

  std::string window_name; ///< video file path
  bool opened; ///< true if opened
  SDL_GLContext sdl_gl_context;
  unsigned int frame_count;
  input_buffer_type dev_type;
  unsigned int width;
  unsigned int height;
  SDL_Window *frame_window;
  SDL_Renderer *frame_renderer;
  TTF_Font *font;
  SDL_Event event;
  s_color font_color;
};
#endif
#endif

