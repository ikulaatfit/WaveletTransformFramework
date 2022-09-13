#include "OpencvImageWriter.h"
#ifdef OPENCV_ENABLED

OpencvImageWriter::OpencvImageWriter()
{
	this->init();
}

OpencvImageWriter::OpencvImageWriter(const std::string &file_path, int width, int height, double fps, std::ostream *error)
{
	this->init();
  this->open(file_path, width, height, fps, error);
}

OpencvImageWriter::~OpencvImageWriter(void)
{
	this->close();
}

void OpencvImageWriter::init()
{
	this->opened = false;
}

bool OpencvImageWriter::isOpened()
{
	return this->opened;
}

bool OpencvImageWriter::open(const std::string &file_path, int width, int height, double fps, std::ostream *error)
{
  this->frame_number = 0;
  size_t ext_sep_pos = file_path.rfind(".");
  if(ext_sep_pos == std::string::npos)
  {
    this->file_path = file_path;
    this->file_ext = ".jpg";
  }
  else
  {
    this->file_path = file_path.substr(0, ext_sep_pos);
    this->file_ext = file_path.substr(ext_sep_pos);
  }
  this->bgra_frame = cv::Mat(height, width, CV_8UC4);
  this->opened = true;
  return true;
}

bool OpencvImageWriter::write(void *bgra_out_frame, std::ostream *error)
{
  if(!this->isOpened())
  {
    if(error != NULL) *error << "Error OpencvImageWriter: Unable to write to unopened file." << std::endl;
    return false;
  }
  this->bgra_frame.data = (unsigned char *)bgra_out_frame;
  std::string frame_number_s = (this->frame_number == 0) ? "" : std::to_string(this->frame_number);
  cv::imwrite(file_path + frame_number_s + file_ext, bgra_frame);
  frame_number++;
  return true;
}

void OpencvImageWriter::close()
{
  this->init();
}
#endif