#ifndef FFMPEG_VIDEO_READER_H
#define FFMPEG_VIDEO_READER_H
#ifdef FFMPEG_ENABLED
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>
}
#include "FileReaderInterface.h"

/**
 * @brief Video getting class
 */
class FFMpegVideoReader: public FileReaderInterface
{
public:
  /**
   * Initialize object and register ffmpeg codec.
   */
  FFMpegVideoReader(bool convert_to_gray);
  /**
   * Initialize object, register ffmpeg codec and open video file.
   * @param v_file video file path
   * @param width width of loaded video or 0 if fail
   * @param height height of loaded video or 0 if fail
   * @param error optional string to write error
   */
  FFMpegVideoReader(const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, bool hw_decode, std::ostream *error = NULL);
  /**
   * Close video file if opened.
   */
	~FFMpegVideoReader();
  /**
   * Return file open state.
   * @return file open state
   */
	bool isOpened();
  /**
   * Open video file.
   * @param v_file video file path
   * @param width width of loaded video or 0 if fail
   * @param height height of loaded video or 0 if fail
   * @param error optional string to write error
   * @return success of open file
   */
	bool openFile(const std::string &v_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error = NULL);
  bool seekToFrame(unsigned int frame_pos, std::ostream *error = NULL);
  /**
   * Close video file if opened.
   */
	void closeFile();
  /**
   * Get next frame.
   * @param bgra_frame output frame
   * @param error optional string to write error
   * @return true if return regular frame or false if reach end of file
   */
	bool getFrame(void *out_frame, std::ostream *error = NULL);
	bool isEof();

	bool hwInit(AVCodecContext *ctx, const enum AVHWDeviceType type, std::ostream *error);
private:
  /**
   * Initialize object and register ffmpeg codec.
   */
  void init();

	std::string v_file; ///< video file path
	bool open; ///< true if opened
  bool repeat;
  AVFormatContext *context;
  AVCodec *video_codec;
  AVStream *video_stream;
  int video_stream_id;
  enum AVPixelFormat hw_pix_fmt;
  enum AVPixelFormat sw_pix_fmt;
  AVFrame *sw_frame;
  AVFrame *frame_out;
  AVFrame *hw_frame;
  AVPacket packet;
  std::vector<unsigned char> frame_out_data;
  unsigned int frame_count;
  SwsContext *sws_context;
  bool eof;
  bool demuxer_eof;
  AVRational fps_int;
  AVCodecContext *codec_context;
  AVBufferRef *hw_device_ctx;
  bool packet_available;
  bool hw_decode;
  unsigned int act_packet_id;
  unsigned int act_send_frame_id;
  unsigned int act_received_frame_id;
  //static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
};

#endif
#endif

