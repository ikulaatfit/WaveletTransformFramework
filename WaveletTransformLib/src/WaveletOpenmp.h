#ifndef WAVELET_OPENMP_H
#define WAVELET_OPENMP_H
#include <string>
#include <climits>
#include "Debug.h"

#include <cmath>
#include <omp.h>
#include "Wavelet.h"
#include "WaveletOpenmpParam.h"

/**
 * @brief Wald boost detector filter class.
 */
class WaveletOpenmp : public Wavelet
{
	public:
    /**
     * Constructor to init detector.
     */
    WaveletOpenmp(rect image_size, WaveletOpenmpParam *param);
    /**
     * Destructor to delete allocated data.
     */
    virtual ~WaveletOpenmp();
    virtual bool isValid();
    /**
    * Transcode frame.
    * @param bgra_frame frame in BGRA format to transcode
    * @param out optional frame process timers
    * @param error optional string to write error
    */
    virtual bool getFrame(uchar4 *bgra_frame, det_output *out = NULL) = 0;
  protected:
    bool getFramePre(uchar4 *bgra_frame);
    bool getFramePost(uchar4 *bgra_frame, float *f_gray_out, float *f_gray_in, det_output *out = NULL);
    /**
     * Delete all buffers and set to initial state.
     */
    void deleteResources();
    void deleteBuffers();
    void deleteTimers();
    /**
     * Try to create buffers and set process state to copy buffers.
     * @param error optional string to write error
     * @return success of create kernel buffers
     */
    bool createResources();
    bool createTimers();
    bool createBuffers();
    /**
    * Try to create statistics dependent buffers and set process state to copy buffers.
    * @param error optional string to write error
    * @return success of create kernel buffers
    */
    void clear();

    void conv_c_rgba_f_gray(float *f_gray, uchar4 *input_frame, size_t width, size_t height, double &time);
    void conv_f_gray_c_rgba(uchar4 *input_frame, float *f_gray, size_t width, size_t height, double &time);

    float *f_gray_image_1;
    float *f_gray_image_2;
    std::vector<double> pre_time;
	std::vector<double> proc_time;
	std::vector<double> post_time;

    WaveletOpenmpParam *param;
};

#endif

