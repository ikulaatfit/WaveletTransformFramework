#include "OpencvCameraReader.h"
#ifdef OPENCV_ENABLED
OpencvCameraReader::OpencvCameraReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

OpencvCameraReader::OpencvCameraReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
	this->init();
	this->openFile(in_file, width, height, fps, repeat, error);
}

OpencvCameraReader::~OpencvCameraReader(void)
{
	this->closeFile();
}

void OpencvCameraReader::init()
{
	this->open = false;
}

bool OpencvCameraReader::isOpened()
	{
		return this->open;
	}

bool OpencvCameraReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  this->closeFile();
  if(!(this->open = video.open(atoi(in_file.c_str()))))
    {
      return false;
    }
  if((width == 0) && (height == 0))
  {
    width = 1920;
    height = 1080;
  }
  //video.set(CV_CAP_PROP_FRAME_WIDTH, width);
  //video.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  video.set(cv::CAP_PROP_FPS, 15);
  width = (unsigned int)(video.get(cv::CAP_PROP_FRAME_WIDTH));
  height = (unsigned int)(video.get(cv::CAP_PROP_FRAME_HEIGHT));
  fps = video.get(cv::CAP_PROP_FPS);
	this->v_file = in_file;
  return true;
}

bool OpencvCameraReader::getFrame(void *out_frame, std::ostream *error)
{
  // warning: CV_CAP_PROP_FRAME_COUNT has sometimes inaccurate value
  if((!this->isOpened()) || (!video.read(bgr_frame)))
  {
    if(error != NULL) *error << "Error: while getting frame.";
    return false;
  }
  void *tmp_bgr_frame = bgra_frame.data;
  bgra_frame.data = (uchar *)out_frame;
  cv::cvtColor(bgr_frame, bgra_frame, (convert_to_gray ? cv::COLOR_BGR2GRAY : cv::COLOR_BGR2BGRA));
  bgra_frame.data = (uchar *)tmp_bgr_frame;
  return true;
}

bool OpencvCameraReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
{
  return true;
}

void OpencvCameraReader::closeFile()
{
  if(this->isOpened())
    {
      video.release();
      this->open = false;
    }
}

bool OpencvCameraReader::isEof()
{
	return false;
}
#endif



