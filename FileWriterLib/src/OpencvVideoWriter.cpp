#include "OpencvVideoWriter.h"
#ifdef OPENCV_ENABLED

OpencvVideoWriter::OpencvVideoWriter()
{
	this->init();
}

OpencvVideoWriter::OpencvVideoWriter(const std::string &file_path, int width, int height, double fps, std::ostream *error)
{
	this->init();
	this->open(file_path, width, height, fps, error);
}

OpencvVideoWriter::~OpencvVideoWriter(void)
{
	this->close();
}

void OpencvVideoWriter::init()
{
}

bool OpencvVideoWriter::isOpened()
{
  return video.isOpened();
}

bool OpencvVideoWriter::open(const std::string &file_path, int width, int height, double fps, std::ostream *error)
{
  bgra_frame = cv::Mat(height, width, CV_8UC4);
  bool opened = video.open(file_path, cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, cv::Size(width, height), true);
  if (opened) return true;
  if(error != NULL) *error << "Warning OpencvVideoWriter: Unable to open file \"" << file_path << "\" with video codec H264. Trying to set MJPG codec." << std::endl;
  opened = video.open(file_path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(width, height), true);
  if (opened) return true;
  if(error != NULL) *error << "Error OpencvVideoWriter: Unable to open file \"" << file_path << "\"." << std::endl;
  return opened;
}

bool OpencvVideoWriter::write(void *bgra_out_frame, std::ostream *error)
{ 
  if(!video.isOpened())
  {
    if(error != NULL) *error << "Error OpencvVideoWriter: Unable to write to unopened file." << std::endl;
    return false;
  }
  bgra_frame.data = (unsigned char *)bgra_out_frame;
  cv::cvtColor(bgra_frame, bgr_frame, cv::COLOR_BGRA2BGR);
  video.write(bgr_frame);
  return true;
}


void OpencvVideoWriter::close()
{
  video.release();
}

#endif