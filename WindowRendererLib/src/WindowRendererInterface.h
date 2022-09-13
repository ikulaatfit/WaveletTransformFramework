#ifndef WINDOW_RENDERER_INTERFACE_H
#define WINDOW_RENDERER_INTERFACE_H

#include <string>
#include <ostream>

enum input_buffer_type
{
  INPUT_BUFFER_TYPE_CPU,
  INPUT_BUFFER_TYPE_OPENGL
};

struct s_color
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
  s_color(unsigned char r=0, unsigned char g=255, unsigned char b=0, unsigned char a=0)
  {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }
};

#define DEF_WINDOW_NUM      -1

/**
 * @brief Interface for getting class
 */
class WindowRendererInterface
{
public:
  /**
   * Close video file if opened.
   */
  virtual ~WindowRendererInterface(){};
  /**
   * Return file open state.
   * @return file open state
   */
  virtual bool isOpened() = 0;
  /**
   * Open video file.
   * @param v_file video file path
   * @param width width of loaded video or 0 if fail
   * @param height height of loaded video or 0 if fail
   * @param error optional string to write error
   * @return success of open file
   */
  virtual bool open(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type = INPUT_BUFFER_TYPE_CPU, int screen = 0, bool fullscreen = false, s_color font_color = s_color(), std::ostream *error = NULL) = 0;
  /**
   * Close video file if opened.
   */
	virtual void close() = 0;
  /**
   * Get next frame.
   * @param bgra_frame output frame
   * @param error optional string to write error
   * @return true if return regular frame or false if reach end of file
   */
  virtual bool showFrame(void *bgra_frame, std::string &output_text, unsigned int text_start_x = 10, unsigned int text_start_y = 10, std::ostream *error = NULL) = 0;
  virtual unsigned char getKey() = 0;
};

#endif

