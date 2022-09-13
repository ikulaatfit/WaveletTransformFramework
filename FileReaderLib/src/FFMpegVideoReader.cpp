#ifdef FFMPEG_ENABLED
#include "FFMpegVideoReader.h"
#include <libavdevice/avdevice.h>
#include <algorithm>
#include <cstring>
#include <thread>
//#include <d3d9helper.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

FFMpegVideoReader::FFMpegVideoReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

FFMpegVideoReader::FFMpegVideoReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, bool hw_decode, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
  this->context = NULL;
  this->video_codec = NULL;
  this->video_stream = NULL;
  this->codec_context = NULL;
  this->sw_frame = NULL;
  this->frame_out = NULL;
  this->hw_frame = NULL;
  this->hw_device_ctx = NULL;
  this->init();
  this->hw_decode = hw_decode;
  this->openFile(in_file, width, height, fps, repeat, error);
}

FFMpegVideoReader::~FFMpegVideoReader(void)
{
	this->closeFile();
}

void FFMpegVideoReader::init()
{
  this->eof = true;
  this->demuxer_eof = true;
  this->open = false;
  if(this->sw_frame != NULL) av_free(sw_frame);
  if(this->frame_out != NULL) av_free(frame_out);
  if (this->hw_frame != NULL) av_free(hw_frame);
  if(this->codec_context != NULL) avcodec_free_context(&codec_context);
  if(this->context != NULL) avformat_close_input(&context);
  if (this->hw_device_ctx != NULL) av_buffer_unref(&hw_device_ctx);
}

bool FFMpegVideoReader::isOpened()
	{
		return this->open;
	}

bool FFMpegVideoReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  int err_msg;
  // open file
  if((err_msg = avformat_open_input(&context, in_file.c_str(), NULL, NULL)) < 0)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot open input file \"" << in_file << "\"." << std::endl;
    return false;
  }
  // get streams from container
  if((err_msg = avformat_find_stream_info(context, NULL)) < 0)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot get stream info \"" << in_file << "\"." << std::endl;
    return false;
  }
  if((video_stream_id = av_find_best_stream(context, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0)) < 0)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error " << video_stream_id << ": Cannot find video stream in file \"" << in_file << "\"." << std::endl;
    return false;
  }

  if (video_codec == NULL)
  {
	  if (error != NULL) *error << "FFMpegVideoReader Error: Cannot find get codec for video stream in file \"" << in_file << "\"." << std::endl;
	  return false;
  }

  if ((codec_context = avcodec_alloc_context3(video_codec)) == NULL)
  {
	  if (error != NULL) *error << "FFMpegVideoReader Error: Cannot allocate context for codec in file \"" << in_file << "\"." << std::endl;
	  return false;
  }

	video_stream = context->streams[video_stream_id];

	if ((err_msg = avcodec_parameters_to_context(codec_context, video_stream->codecpar)) < 0)
	{
		if (error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot create context from parameters in file \"" << in_file << "\"." << std::endl;
		return false;
	}
	/**/
	AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
	if (hw_decode)
	{
		for (int i = 0;; i++)
		{
			const AVCodecHWConfig *config;
			if ((config = avcodec_get_hw_config(video_codec, i)) == NULL)
			{
				if (error != NULL) *error << "FFMpegVideoReader Note: Cannot get hw config for codec " << video_codec->name << " using framework " << av_hwdevice_get_type_name(type) << "." << std::endl;
				type = AV_HWDEVICE_TYPE_NONE;
				break;
			}
			if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX/* && config->device_type == type*/) {
				sw_pix_fmt = AV_PIX_FMT_NV12;
				hw_pix_fmt = config->pix_fmt;
				type = config->device_type;
				if (!this->hwInit(codec_context, type, error)) continue;
				codec_context->thread_count = std::min((int)std::thread::hardware_concurrency(), 4);
				if (error != NULL) *error << "FFMpegVideoReader Note: Codec " << video_codec->name << " open using " << av_hwdevice_get_type_name(config->device_type) << " hw decoder." << std::endl;
				break;
			}
		}
	}
	if(type == AV_HWDEVICE_TYPE_NONE)
	/**/
	{
		sw_pix_fmt = codec_context->pix_fmt;
		hw_pix_fmt = AV_PIX_FMT_NONE;
		codec_context->thread_count = std::thread::hardware_concurrency();
		if (error != NULL) *error << "FFMpegVideoReader Note: Codec " << video_codec->name << " open using software decoder." << std::endl;
	}
  if((err_msg = avcodec_open2(codec_context, video_codec, NULL)) < 0)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot open codec for video stream in file \"" << in_file << "\"." << std::endl;
    return false;
  }
  if((sw_frame = av_frame_alloc()) == NULL)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error: Cannot get frame in file \"" << in_file << "\"." << std::endl;
    return false;
  }
  if((frame_out = av_frame_alloc()) == NULL)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error: Cannot get frame in file \"" << in_file << "\"." << std::endl;
    return false;
  }

  if ((hw_frame = av_frame_alloc()) == NULL)
  {
	  if (error != NULL) *error << "FFMpegVideoReader Error: Cannot get frame in file \"" << in_file << "\"." << std::endl;
	  return false;
  }

  frame_out_data.resize(av_image_get_buffer_size((convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA, codec_context->width, codec_context->height,1));
  //if((err_msg = avpicture_fill((AVPicture *)frame_out, frame_out_data.data(), AV_PIX_FMT_GRAY8, codec_context->width, codec_context->height)) < 0)
  if ((err_msg = av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_out_data.data(), (convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA, codec_context->width, codec_context->height, 1)) < 0)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot get frame in file \"" << in_file << "\"." << std::endl;
    return false;
  }
  frame_out->width = codec_context->width;
  frame_out->height = codec_context->height;
  frame_out->format = (convert_to_gray) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA;
  

  if((codec_context->width <= 0) || (codec_context->height <= 0))
  {
    if(error != NULL) *error << "FFMpegVideoReader Error: Invalid size of video \"" << in_file << "\"." << std::endl;
    return false;
  }

  if((sws_context = sws_getContext(codec_context->width, codec_context->height, sw_pix_fmt,
                                   codec_context->width, codec_context->height, (convert_to_gray)?AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGRA, SWS_FAST_BILINEAR,
                                   NULL, NULL, NULL)) == NULL)
  {
    if(error != NULL) *error << "FFMpegVideoReader Error: Cannot open resize context for file \"" << in_file << "\"." << std::endl;
    return false;
  }

  av_init_packet(&packet);

  width = codec_context->width;
  height = codec_context->height;
  fps = av_q2d(video_stream->r_frame_rate);
  if(fps == 0.0)
  {
    fps = 30.0;
    if(error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot get frame in file \"" << in_file << "\"." << std::endl;
  }
  this->open = true;
  this->eof = false;
  this->demuxer_eof = false;
  this->packet_available = false;

  this->act_packet_id = 0;
  this->act_send_frame_id = 0;
  this->act_received_frame_id = 0;
  return true;
}

bool FFMpegVideoReader::getFrame(void *out_frame, std::ostream *error)
{
	bool packet_read = true;
	int err_msg;
	while (1)
	{
		while ((packet_available) || (packet_read = ((err_msg = av_read_frame(context, &packet)) == 0)) || (!this->demuxer_eof))
		{
			if (!packet_read) // create terminal packet for enable of receiving rest of send packets
			{
				packet.data = NULL;
				packet.size = 0;
				this->demuxer_eof = true;
			}
#ifdef DEBUG_FILE_READER
			if (!packet_available)
			{
				if (error != NULL) std::cerr << "FFMpegVideoReader Debug: Demuxer: packet id " << this->act_packet_id << "." << std::endl;
				act_packet_id++;
			}
#endif
			packet_available = false;
			if ((packet.stream_index != video_stream_id) && (!this->demuxer_eof))
			{
#ifdef DEBUG_FILE_READER
				if (error != NULL) std::cerr << "FFMpegVideoReader Debug: Demuxer: Packet id " << this->act_packet_id << " is not a video stream packet." << std::endl;
#endif
				av_packet_unref(&packet);
				continue;
			}

			if ((err_msg = avcodec_send_packet(codec_context, &packet)) < 0)
			{
				if (err_msg == AVERROR(EAGAIN))
				{
#ifdef DEBUG_FILE_READER
					if (error != NULL) std::cerr << "FFMpegVideoReader Debug: Decoder: Packet id " << this->act_send_frame_id << " cannot be send to decoder. Decoder is full." << std::endl;
#endif
					this->packet_available = true;
					break;
				}
				else if (err_msg == AVERROR_EOF)
				{
#ifdef DEBUG_FILE_READER
					if (error != NULL) std::cerr << "FFMpegVideoReader Debug: Decoder: Packet id " << this->act_send_frame_id << " cannot be send to decoder. Decored is in EOF state." << std::endl;
#endif
					break;
				}
				if (error != NULL) *error << "FFMpegVideoReader Warning " << err_msg << ": Error while sending packet. Packet skipped." << std::endl;
				av_packet_unref(&packet);
				continue;
			}
			
			av_packet_unref(&packet);
#ifdef DEBUG_FILE_READER
			if (error != NULL) *error << "FFMpegVideoReader Debug: Decoder: Frame id " << act_send_frame_id << " is send." << std::endl;
			act_send_frame_id++;
#endif
		}
		if ((err_msg = avcodec_receive_frame(codec_context, hw_frame)) < 0)
		{
			if (err_msg == AVERROR_EOF)
			{
				this->eof = true;
				if (error != NULL) *error << "FFMpegVideoReader Note: Video reached end of file. Terminating." << std::endl;
				return false;
			}
			else if (err_msg == AVERROR(EAGAIN))
			{
				if (error != NULL) *error << "FFMpegVideoReader Note: Video decoder has empty queue. Queue need to by filled byt send packet." << std::endl;
				return false;
			}
			if (error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Error while receiving packet. Packet skipped." << std::endl;
			continue;
		}
#ifdef DEBUG_FILE_READER
		if (error != NULL) *error << "FFMpegVideoReader Debug: Frame id " << act_received_frame_id << " is received." << std::endl;
		act_received_frame_id++;
#endif

		AVFrame *tmp_frame = hw_frame;
		/**/
		if (hw_frame->format == this->hw_pix_fmt)
		{
			if ((err_msg = av_hwframe_transfer_data(sw_frame, hw_frame, 0)) < 0)
			{
				if (error != NULL) *error << "FFMpegVideoReader Error " << err_msg << ": Cannot copy frame to host. Packet skipped." << std::endl;
				continue;
			}
			tmp_frame = sw_frame;
		}
		/**/
		if (((tmp_frame->linesize[0] == frame_out->width) && ((tmp_frame->format == AV_PIX_FMT_NV12) || (tmp_frame->format == AV_PIX_FMT_YUV420P) || (tmp_frame->format == AV_PIX_FMT_NV21)) || (tmp_frame->format == AV_PIX_FMT_YUVJ444P)) && (this->convert_to_gray))
		{
			std::memcpy(out_frame, tmp_frame->data[0], frame_out->width * frame_out->height);
		}
		else
		{
			if ((tmp_frame->format != sw_pix_fmt) || (tmp_frame->width != frame_out->width) || (tmp_frame->height != frame_out->height))
			{
				if (error != NULL) *error << "FFMpegVideoReader Error: Cannot convert packet to output format. Packet skipped." << std::endl;
				continue;
			}
			if (sws_scale(sws_context, tmp_frame->data, tmp_frame->linesize, 0, tmp_frame->height, (uint8_t * const*)&out_frame, frame_out->linesize) <= 0)
			{

				if (error != NULL) *error << "FFMpegVideoReader Warning: Cannot convert packet to output format. Packet skipped." << std::endl;
				continue;
			}
		}
		return true;
	}
	return false;
}

bool FFMpegVideoReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
{
	if (!isOpened()) return false;
	int err_msg;
	//std::cerr << video_stream->time_base.den << " " << video_stream->time_base.num << " " << video_stream->r_frame_rate.den << " " << video_stream->r_frame_rate.num << std::endl;
	uint64_t frame_based_time = ((uint64_t)video_stream->time_base.den) * ((uint64_t)video_stream->r_frame_rate.den) * ((uint64_t)frame_pos) / (((uint64_t)video_stream->time_base.num)*((uint64_t)video_stream->r_frame_rate.num));
	if ((err_msg = av_seek_frame(context, video_stream_id, frame_based_time, 0)) < 0)
	//if ((err_msg = avformat_seek_file(context, video_stream_id, frame_based_time, frame_based_time, frame_based_time, 0)) < 0)
	{
		if (error != NULL) *error << "FFMpegVideoReader Error: Unknown FFmpeg error number " << err_msg << " while seeking to frame_pos " << frame_pos << " Terminating." << std::endl;
		return false;
	}
	return true;
}

void FFMpegVideoReader::closeFile()
{
  this->init();
}

bool FFMpegVideoReader::isEof()
{
	return eof;
}

bool FFMpegVideoReader::hwInit(AVCodecContext *ctx, const enum AVHWDeviceType type, std::ostream *error)
{
	int err_msg = 0;

	/**/
	if ((err_msg = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0)) < 0) {
		if (error != NULL) *error << "FFMpegVideoReader Note: Unable to create device context using " << av_hwdevice_get_type_name(type) << "." << std::endl;
		return false;
	}
	ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
	/**/

	return true;
}

#endif
