#ifndef FILE_WRITER_INTERFACE_H
#define FILE_WRITER_INTERFACE_H
#include <string>
#include <ostream>

/**
 * @brief Interface for writing class
 */
class FileWriterInterface
{
public:
  /**
  * Close opened file.
  */
	virtual ~FileWriterInterface(){};
  
  /**
  * Obtain file state.
  * @return file state
  */
  virtual bool isOpened() = 0;

  /**
  * Open video file.
  * @param file_path file path of video/image
  * @param width width of video/image
  * @param height height of video/image
  * @param fps framerate of video/image
  * @param error optional stream for error writing
  * @return true if successfully opened
  */
	virtual bool open(const std::string &file_path, int width, int height, double fps, std::ostream *error = NULL) = 0;
  
  /**
   * Close video file if opened.
   */
	virtual void close() = 0;

  /**
  * Write frame.
  * @param data data to write
  * @param error optional stream for error writing
  * @return success
  */
	virtual bool write(void *data, std::ostream *error = NULL) = 0;
};

#endif

