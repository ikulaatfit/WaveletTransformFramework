#ifndef NONE_READER_H
#define NONE_READER_H
#include <iostream>
#include <string>
#include <sstream>

#include "FileReaderInterface.h"

/**
 * @brief Video getting class
 */
class NoneReader : public FileReaderInterface
{
public:
  /**
   * Initialize object and register ffmpeg codec.
   */
  NoneReader(bool convert_to_gray);
  /**
   * Initialize object, register ffmpeg codec and open video file.
   * @param v_file video file path
   * @param width width of loaded video or 0 if fail
   * @param height height of loaded video or 0 if fail
   * @param error optional string to write error
   */
  NoneReader(const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error = NULL);
  /**
   * Close video file if opened.
   */
  ~NoneReader();
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
	bool openFile(const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error = NULL);
  bool seekToFrame(unsigned int frame_pos, std::ostream *error = NULL);
  /**
   * Close video file if opened.
   */
	void closeFile();
  /**
   * Get next frame.
   * @param bgra_frame output frame
   * @param error optional string to write error
   * @return true if return regular frame or false if reach end of file
   */
	bool getFrame(void *out_frame, std::ostream *error = NULL);
	bool isEof();
private:
  /**
   * Initialize object and register ffmpeg codec.
   */
  void init();

	bool open;
};

#endif

