#include "OpencvVideoReader.h"
#ifdef OPENCV_ENABLED

OpencvVideoReader::OpencvVideoReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

OpencvVideoReader::OpencvVideoReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
	this->init();
	this->openFile(in_file, width, height, fps, repeat, error);
}

OpencvVideoReader::~OpencvVideoReader(void)
{
	this->closeFile();
}

void OpencvVideoReader::init()
{
	this->open = false;
}

bool OpencvVideoReader::isOpened()
	{
		return this->open;
	}

bool OpencvVideoReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  this->closeFile();
  if(!(this->open = video.open(in_file)))
  {
    return false;
  }
  this->frame_count = (unsigned int)(video.get(cv::CAP_PROP_FRAME_COUNT));
  width = (unsigned int)(video.get(cv::CAP_PROP_FRAME_WIDTH));
  height = (unsigned int)(video.get(cv::CAP_PROP_FRAME_HEIGHT));
  fps = video.get(cv::CAP_PROP_FPS);
	this->v_file = in_file;
  this->repeat = repeat;
  return true;
}

bool OpencvVideoReader::getFrame(void *out_frame, std::ostream *error)
{
  // warning: CV_CAP_PROP_FRAME_COUNT has sometimes inaccurate value
  if((!this->isOpened()) ||
      ((!video.read(bgr_frame)) &&
      ((!this->repeat) || (!this->seekToFrame(0)) || (!video.read(bgr_frame)))))
  {
    return false;
  }
  void *tmp_bgr_frame = bgra_frame.data;
  bgra_frame.data = (uchar *)out_frame;
  cv::cvtColor(bgr_frame, bgra_frame, (convert_to_gray ? cv::COLOR_BGR2GRAY : cv::COLOR_BGR2BGRA));
  bgra_frame.data = (uchar *)tmp_bgr_frame;
  return true;
}

bool OpencvVideoReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
{
  if((!this->isOpened()) || (this->frame_count <= frame_pos) || (!video.set(cv::CAP_PROP_POS_FRAMES, (double)frame_pos)))
  {
    return false;
  }
  return true;
}

void OpencvVideoReader::closeFile()
{
  if(this->isOpened())
  {
    video.release();
    this->open = false;
  }
}

bool OpencvVideoReader::isEof()
{
	return false;
}

#endif