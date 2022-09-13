#ifndef FILE_READER_INTERFACE_H
#define FILE_READER_INTERFACE_H
#include <string>
#include <ostream>

/**
 * @brief Interface for getting class
 */
class FileReaderInterface
{
public:
  /**
   * Close video file if opened.
   */

	FileReaderInterface(bool convert_to_gray_in):convert_to_gray(convert_to_gray_in){};
	virtual ~FileReaderInterface(){};
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
   * @param fps video framerate
   * @param repeat repeat video/image processing
   * @param error optional string to write error
   * @return success of open file
   */
	virtual bool openFile(const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error = NULL) = 0;
  /**
   * Close video file if opened.
   */
	virtual void closeFile() = 0;

  virtual bool seekToFrame(unsigned int frame_pos, std::ostream *error = NULL) = 0;
  /**
   * Get next frame.
   * @param bgra_frame output frame
   * @param error optional string to write error
   * @return true if return regular frame or false if reach end of file
   */
	virtual bool getFrame(void *out_frame, std::ostream *error = NULL) = 0;

	virtual bool isEof() = 0;
	bool convert_to_gray;
};

#endif

