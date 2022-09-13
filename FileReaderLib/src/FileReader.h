#ifndef FILE_READER_H
#define FILE_READER_H

#include <vector>
#include "EmptyReader.h"
#include "NoneReader.h"

#ifdef OPENCV_ENABLED
#include "OpencvCameraReader.h"
#include "OpencvImageReader.h"
#include "OpencvVideoReader.h"
#endif

#ifdef FFMPEG_ENABLED
#include "FFMpegVideoReader.h"
#include "FFMpegCameraReader.h"
#endif

#include "FileReaderInterface.h"

enum e_reader_type
{
  READER_TYPE_CAMERA,
  READER_TYPE_VIDEO,
  READER_TYPE_IMAGE,
#ifdef OPENCV_ENABLED
  READER_TYPE_OPENCV_CAMERA,
  READER_TYPE_OPENCV_IMAGE,
  READER_TYPE_OPENCV_VIDEO,
#endif
#ifdef FFMPEG_ENABLED
  READER_TYPE_FFMPEG_VIDEO,
  READER_TYPE_FFMPEG_CAMERA,
#endif
  READER_TYPE_NONE,
  READER_TYPE_EMPTY,
  READER_TYPE_INVALID
};

inline e_reader_type get_reader_type(const std::string &filepath)
{
  size_t ext_start = filepath.find_last_of(".") + 1;
  if (ext_start == 0) return READER_TYPE_CAMERA;
  std::string out_file_ext = filepath.substr(ext_start);
  std::vector<const char *> video_exts = {"avi", "mkv", "mp4"};
  std::vector<const char *> image_exts = { "jpeg", "jpg", "png", "tiff" };
  for(int i = 0; i < video_exts.size(); i++)
  {
    if(out_file_ext.compare(video_exts[i]) == 0) return READER_TYPE_VIDEO;
  }
  for (int i = 0; i < image_exts.size(); i++)
  {
	  if (out_file_ext.compare(image_exts[i]) == 0) return READER_TYPE_IMAGE;
  }
  return READER_TYPE_INVALID;
}

inline FileReaderInterface *createFileReader(e_reader_type reader_type, const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, bool hw_decode, std::ostream *error = NULL)
{
  FileReaderInterface *out_interface = NULL;
  switch(reader_type)
  {
  case READER_TYPE_CAMERA:
	#ifdef FFMPEG_ENABLED
		 out_interface = new FFMpegCameraReader(v_file, width, height, fps, repeat, convert_to_gray, error);
		 if (out_interface->isOpened()) break;
	#endif
	#ifdef OPENCV_ENABLED
		 out_interface = new OpencvCameraReader(v_file, width, height, fps, repeat, convert_to_gray, error);
		 if (out_interface->isOpened()) break;
	#endif
    break;
  case READER_TYPE_IMAGE:
	#ifdef OPENCV_ENABLED
		out_interface = new OpencvImageReader(v_file, width, height, fps, repeat, convert_to_gray, error);
		if (out_interface->isOpened()) break;
	#endif
    break;
  case READER_TYPE_VIDEO:
#ifdef FFMPEG_ENABLED
	out_interface = new FFMpegVideoReader(v_file, width, height, fps, repeat, convert_to_gray, hw_decode, error);
	if (out_interface->isOpened()) break;
#endif
#ifdef OPENCV_ENABLED
    out_interface = new OpencvVideoReader(v_file, width, height, fps, repeat, convert_to_gray, error);
	if (out_interface->isOpened()) break;
#endif
    break;
#ifdef FFMPEG_ENABLED
  case READER_TYPE_FFMPEG_VIDEO:
	  out_interface = new FFMpegVideoReader(v_file, width, height, fps, repeat, convert_to_gray, hw_decode, error);
	  break;
  case READER_TYPE_FFMPEG_CAMERA:
	  out_interface = new FFMpegCameraReader(v_file, width, height, fps, repeat, convert_to_gray, error);
	  break;
#endif
#ifdef OPENCV_ENABLED
  case READER_TYPE_OPENCV_VIDEO:
	  out_interface = new OpencvVideoReader(v_file, width, height, fps, repeat, convert_to_gray, error);
	  break;
  case READER_TYPE_OPENCV_CAMERA:
	  out_interface = new OpencvCameraReader(v_file, width, height, fps, repeat, convert_to_gray, error);
	  break;
  case READER_TYPE_OPENCV_IMAGE:
	  out_interface = new OpencvImageReader(v_file, width, height, fps, repeat, convert_to_gray, error);
	  break;
#endif
  case READER_TYPE_EMPTY:
    out_interface = new EmptyReader(v_file, width, height, fps, repeat, convert_to_gray, error);
    break;
  case READER_TYPE_NONE:
    out_interface = new NoneReader(v_file, width, height, fps, repeat, convert_to_gray, error);
    break;
  case READER_TYPE_INVALID:
	if (error != NULL) *error << "File reader info: Invalid file.";
	break;
  }
  if(!out_interface->isOpened())
  {
    delete out_interface;
    if(error != NULL) *error << "File reader info: Cannot open file reader.";
    return NULL;
  }
    
  return out_interface;
}

inline FileReaderInterface *createFileReader(const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, bool hw_decode, std::ostream *error = NULL)
{
  return createFileReader(get_reader_type(v_file), v_file, width, height, fps, repeat, convert_to_gray, hw_decode, error);
}

#endif

