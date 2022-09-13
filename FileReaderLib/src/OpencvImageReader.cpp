#include "OpencvImageReader.h"
#ifdef OPENCV_ENABLED
OpencvImageReader::OpencvImageReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

OpencvImageReader::OpencvImageReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
	this->init();
	this->openFile(in_file, width, height, fps, repeat, error);
}

OpencvImageReader::~OpencvImageReader(void)
{
	this->closeFile();
}

void OpencvImageReader::init()
{
	this->open = false;
  this->first = false;
}

bool OpencvImageReader::isOpened()
	{
		return this->open;
	}

bool OpencvImageReader::isReady()
  {
    return this->first && this->isOpened();
  }

bool OpencvImageReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  this->closeFile();
  bgr_frame = cv::imread(in_file);
  if((bgr_frame.data == NULL) || (bgr_frame.dims > 2))
    {
      return false;
    }
  width = bgr_frame.cols;
  height = bgr_frame.rows;
  fps = 30;
  this->open = true;
  this->first = true;
  this->repeat = repeat;
  return true;
}

bool OpencvImageReader::getFrame(void *out_frame, std::ostream *error)
{
  if(!this->isOpened())
  {
    if(error != NULL) *error << "Error: while getting frame.";
    return false;
  }
  if(!this->first)
  {
    if((!repeat) || (!this->seekToFrame(0)))
    {
      return false;
    }
  }
  void *tmp_bgr_frame = bgra_frame.data;
  bgra_frame.data = (uchar *)out_frame;
  cv::cvtColor(bgr_frame, bgra_frame, (convert_to_gray ? cv::COLOR_BGR2GRAY : cv::COLOR_BGR2BGRA));
  bgra_frame.data = (uchar *)tmp_bgr_frame;
  this->first = false;
  return true;
}

bool OpencvImageReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
  {
    if((!this->isOpened()) || (frame_pos != 0))
      {
        return false;
      }
    this->first = true;
    return true;
  }

void OpencvImageReader::closeFile()
{
  this->init();
}

bool OpencvImageReader::isEof()
{
	return false;
}

#endif