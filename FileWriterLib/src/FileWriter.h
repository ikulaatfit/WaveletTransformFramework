#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#ifdef OPENCV_ENABLED
#include "OpencvImageWriter.h"
#include "OpencvVideoWriter.h"
#endif

#include "FileWriterInterface.h"

enum e_writer_type
{
  WRITER_TYPE_IMAGE,
  WRITER_TYPE_VIDEO,
#ifdef OPENCV_ENABLED
  WRITER_TYPE_OPENCV_IMAGE,
  WRITER_TYPE_OPENCV_VIDEO,
#endif
};

inline e_writer_type get_writer_type(const std::string &filepath)
{
  size_t ext_start = filepath.find_last_of(".") + 1;
  std::string out_file_ext = filepath.substr((ext_start == 0) ? filepath.size() : ext_start);
  std::vector<const char *> video_exts = {"avi", "mkv", "mp4"};
  bool video_type = false;
  for(int i = 0; i < video_exts.size(); i++)
  {
    if(out_file_ext.compare(video_exts[i]) == 0) video_type = true;
  }
  return video_type ? WRITER_TYPE_VIDEO : WRITER_TYPE_IMAGE;
}

inline FileWriterInterface *createFileWriter(e_writer_type writer_type, const std::string &file_path, int width, int height, double fps, std::ostream *error = NULL)
{
  FileWriterInterface *out_interface = NULL;
  switch(writer_type)
  {
  case WRITER_TYPE_IMAGE:
#ifdef OPENCV_ENABLED
    out_interface = new OpencvImageWriter(file_path, width, height, fps, error);
	if (out_interface->isOpened()) break;
#endif
    break;
  case WRITER_TYPE_VIDEO:
#ifdef OPENCV_ENABLED
    out_interface = new OpencvVideoWriter(file_path, width, height, fps, error);
	if (out_interface->isOpened()) break;
#endif
    break;
#ifdef OPENCV_ENABLED
  case WRITER_TYPE_OPENCV_IMAGE:
	  out_interface = new OpencvImageWriter(file_path, width, height, fps, error);
	  break;
  case WRITER_TYPE_OPENCV_VIDEO:
	  out_interface = new OpencvVideoWriter(file_path, width, height, fps, error);
	  break;
#endif
  }
  if(!out_interface->isOpened())
  {
    delete out_interface;
    if(error != NULL) *error << "Error FileWriter: Unable open FileWriter." << std::endl;
    return NULL;
  }

  return out_interface;
}

inline FileWriterInterface *createFileWriter(const std::string &file_path, int width, int height, double fps, std::ostream *error = NULL)
{
  return createFileWriter(get_writer_type(file_path), file_path, width, height, fps, error);
}

#endif

