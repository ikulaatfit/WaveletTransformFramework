#include "OpencvRenderer.h"
#ifdef OPENCV_ENABLED
OpencvRenderer::OpencvRenderer()
{
	this->init();
}

OpencvRenderer::OpencvRenderer(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type, int screen, bool fullscreen, s_color font_color, std::ostream *error)
{
	this->init();
  this->open(window_name, width, height, dev_type, screen, fullscreen, font_color, error);
}

OpencvRenderer::~OpencvRenderer(void)
{
	this->close();
}

void OpencvRenderer::init()
{
	this->opened = false;
}

bool OpencvRenderer::isOpened()
{
	return this->opened;
}

bool OpencvRenderer::open(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type, int screen, bool fullscreen, s_color font_color, std::ostream *error)
{
  this->close();
  this->dev_type = dev_type;
  this->window_name = window_name;
  switch(dev_type)
  {
    case INPUT_BUFFER_TYPE_CPU:
      cv::namedWindow(this->window_name.c_str(), cv::WINDOW_NORMAL | cv::WINDOW_AUTOSIZE);
      break;
    default:
      if(error != NULL) *error << "Error: Invalid output type.";
      return false;
      exit(4);
  }
  this->frame_image_2 = cv::Mat(height, width, CV_8UC4);
  this->width = width;
  this->height = height;
  this->opened = true;
  this->font_color = font_color;
  return true;
}

bool OpencvRenderer::showFrame(void *bgra_frame, std::string &output_text, unsigned int text_start_x, unsigned int text_start_y, std::ostream *error)
{
  if(!this->isOpened())
  {
    return false;
  }
  this->frame_image_2.data = (unsigned char *)bgra_frame;
  if (output_text.length() != 0)
  {
	  std::istringstream output_stream(output_text);
	  std::string act_data;
	  for (int act_pos_y = text_start_y; std::getline(output_stream, act_data); act_pos_y += 20)
	  {
		  cv::putText(this->frame_image_2, act_data.c_str(), cv::Point(text_start_x, act_pos_y), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(font_color.b, font_color.g, font_color.r, font_color.a));
	  }
  }
  cv::imshow(this->window_name, this->frame_image_2);
  return true;
}

void OpencvRenderer::close()
{
  if(this->isOpened())
  {
    cv::destroyWindow(this->window_name);
    this->opened = false;
  }
}

unsigned char OpencvRenderer::getKey()
{
  if(!this->isOpened()) return 0;
  unsigned int key;
  if((key = cv::waitKey(1) & 255) >= 0)
  {
    return (unsigned char)key;
  }
  return 0;
}

#endif