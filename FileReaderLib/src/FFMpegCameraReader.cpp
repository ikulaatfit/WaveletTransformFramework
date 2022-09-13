#ifdef FFMPEG_ENABLED
#include "FFMpegCameraReader.h"

FFMpegCameraReader::FFMpegCameraReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

FFMpegCameraReader::FFMpegCameraReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
  this->context = NULL;
  this->video_codec = NULL;
  this->video_stream = NULL;
  this->codec_context = NULL;
  this->frame = NULL;
  this->frame_out = NULL;
	this->init();
	this->openFile(in_file, width, height, fps, repeat, error);
}

FFMpegCameraReader::~FFMpegCameraReader(void)
{
	this->closeFile();
}

void FFMpegCameraReader::init()
{
  this->open = false;
  if(this->frame != NULL) av_free(frame);
  if(this->frame_out != NULL) av_free(frame_out);
  if(this->codec_context != NULL)
  if(this->codec_context != NULL) avcodec_free_context(&codec_context);
  if(this->context != NULL) avformat_close_input(&context);
}

bool FFMpegCameraReader::isOpened()
	{
		return this->open;
	}

bool FFMpegCameraReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  int err_msg;
  std::string test("video=A4 TECH HD PC Camera");

  av_register_all();
  avdevice_register_all();
  AVInputFormat *input_format;
  AVDictionary* opt = NULL;
  av_dict_set(&opt, "video_size", "640x480", 0);
  if((input_format = av_find_input_format("dshow")) == NULL)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error: Cannot open input format file \"" << in_file << "\"." << std::endl;
    return false;
  }
  // open file;
  if((err_msg = avformat_open_input(&context, test.c_str(), input_format, &opt)) < 0)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error " << err_msg << ": Cannot open input file \"" << in_file << "\"." << std::endl;
    return false;
  }

  // get streams from container
  if((err_msg = avformat_find_stream_info(context, NULL)) < 0)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error " << err_msg << ": Cannot get stream info \"" << in_file << "\"." << std::endl;
    return false;
  }

  if((video_stream_id = av_find_best_stream(context, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0)) < 0)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error " << video_stream_id << ": Cannot find video stream in file \"" << in_file << "\"." << std::endl;
    return false;
  }

  if(video_codec == NULL)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error: Cannot find get codec for video stream in file \"" << in_file << "\"." << std::endl;
    return false;
  }

  if ((codec_context = avcodec_alloc_context3(video_codec)) == NULL)
  {
	  if (error != NULL) *error << "FFMpegCameraReader Error: Cannot allocate context for codec in file \"" << in_file << "\"." << std::endl;
	  return false;
  }

  video_stream = context->streams[video_stream_id];

  if ((err_msg = avcodec_parameters_to_context(codec_context, video_stream->codecpar)) < 0)
  {
	  if (error != NULL) *error << "FFMpegCameraReader Error " << err_msg << ": Cannot create context from parameters in file \"" << in_file << "\"." << std::endl;
	  return false;
  }

  if((err_msg = avcodec_open2(codec_context, video_codec, NULL)) < 0)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error " << err_msg << ": Cannot open codec for video stream in file \"" << in_file << "\"." << std::endl;
    return false;
  }
  if((frame = av_frame_alloc()) == NULL)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error: Cannot get frame in file \"" << in_file << "\"." << std::endl;
    return false;
  }
  if((frame_out = av_frame_alloc()) == NULL)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error: Cannot get frame in file \"" << in_file << "\"." << std::endl;
    return false;
  }

  frame_out_data.resize(av_image_get_buffer_size((convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA, codec_context->width, codec_context->height, 1));
  //if((err_msg = avpicture_fill((AVPicture *)frame_out, frame_out_data.data(), AV_PIX_FMT_BGRA, codec_context->width, codec_context->height)) < 0)
  if ((err_msg = av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_out_data.data(), (convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA, codec_context->width, codec_context->height, 1)) < 0)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error " << err_msg << ": Cannot get frame in file \"" << in_file << "\"." << std::endl;
    return false;
  }
  frame_out->width = codec_context->width;
  frame_out->height = codec_context->height;
  frame_out->format = (convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA;
  

  if((codec_context->width <= 0) || (codec_context->height <= 0))
  {
    if(error != NULL) *error << "FFMpegCameraReader Error: Invalid size of video \"" << in_file << "\"." << std::endl;
    return false;
  }

  if((sws_context = sws_getContext(codec_context->width, codec_context->height, codec_context->pix_fmt,
                                   codec_context->width, codec_context->height, (convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA, SWS_BICUBIC,
                                   NULL, NULL, NULL)) == NULL)
  {
    if(error != NULL) *error << "FFMpegCameraReader Error: Cannot open resize context for file \"" << in_file << "\"." << std::endl;
    return false;
  }

  av_init_packet(&packet);

  width = codec_context->width;
  height = codec_context->height;
  fps = av_q2d(video_stream->r_frame_rate);
  if(fps == 0.0)
  {
    fps = 30.0; // implicit value
    if(error != NULL) *error << "FFMpegCameraReader Error " << err_msg << ": Cannot get frame in file \"" << in_file << "\"." << std::endl;
  }
  this->open = true;
  return true;
}

bool FFMpegCameraReader::getFrame(void *out_frame, std::ostream *error)
{
  int err_msg;
  while(av_read_frame(context, &packet) == 0)
  {
    if(packet.stream_index != video_stream_id) continue;
    int is_valid;

    if((err_msg = avcodec_decode_video2(video_stream->codec, frame, &is_valid, &packet)) < 0)
    {
      if(error != NULL) *error << "FFMpegCameraReader Warning "<< err_msg << ": Error while reading packet. Packet skipped." << std::endl;
      continue;
    }

    if(!is_valid)
    {
      if(error != NULL) *error << "FFMpegCameraReader Note " << err_msg << ": Invalid packet. Packet skipped." << std::endl;
	  av_packet_unref(&packet);
      continue;
    }
    if(sws_scale(sws_context, frame->data, frame->linesize, 0, frame->height, (uint8_t * const*)&out_frame, frame_out->linesize) <= 0)
    {
      if(error != NULL) *error << "FFMpegCameraReader Warning: Cannot convert packet to RGBA. Packet skipped." << std::endl;
	  av_packet_unref(&packet);
      continue;
    }
	av_packet_unref(&packet);
    break;
  }
  return true;
}

bool FFMpegCameraReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
{
    return false;
}

void FFMpegCameraReader::closeFile()
{
  this->init();
}

bool FFMpegCameraReader::isEof()
{
	return false;
}
#endif