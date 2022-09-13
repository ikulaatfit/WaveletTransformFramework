#ifndef OPENCV_VIDEO_WRITER_H
#define OPENCV_VIDEO_WRITER_H
#ifdef OPENCV_ENABLED
#include <iostream>
#include <string>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "FileWriterInterface.h"


/**
* @brief Video writer class
*/
class OpencvVideoWriter: public FileWriterInterface
{
public:
  /**
  * Initialize object
  */
	OpencvVideoWriter();

  /**
  * Initialize object and open file
  * @param file_path file path of video
  * @param width width of video
  * @param height height of video
  * @param fps framerate of video
  * @param error optional stream for error writing
  */
	OpencvVideoWriter(const std::string &file_path, int width, int height, double fps, std::ostream *error = NULL);

  /**
   * Close video file if opened.
   */
	~OpencvVideoWriter();

  /**
   * Obtain file state.
   * @return file state
   */
	bool isOpened();

  /**
  * Open video file.
  * @param file_path file path of video
  * @param width width of video
  * @param height height of video
  * @param fps framerate of video
  * @param error optional stream for error writing
  * @return true if successfully opened
  */
	bool open(const std::string &file_path, int width, int height, double fps, std::ostream *error = NULL);
  
  /**
  * Close opened video file.
  */
	void close();
  
  /**
  * Write frame.
  * @param data data to write
  * @param error optional stream for error writing
  * @return success
  */
	bool write(void *data, std::ostream *error = NULL);
private:
  
  /**
  * Initialize object.
  */
  void init();

  cv::VideoWriter video; ///< video writer object
  cv::Mat bgra_frame; ///< empty image struct
  cv::Mat bgr_frame;
};
#endif
#endif

