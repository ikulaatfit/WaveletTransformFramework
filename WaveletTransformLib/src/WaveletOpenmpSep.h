#ifndef WAVELET_OPENMP_SEP_H
#define WAVELET_OPENMP_SEP_H
#include <string>
#include <climits>
#include "Debug.h"
#include <cmath>
#include <omp.h>
#include "WaveletOpenmp.h"
#include "WaveletOpenmpParamSep.h"

/**
 * @brief Wald boost detector filter class.
 */
class WaveletOpenmpSep : public WaveletOpenmp
{
	public:
    /**
     * Constructor to init detector.
     */
    WaveletOpenmpSep(rect image_size, WaveletOpenmpParamSep *param);
    /**
     * Destructor to delete allocated data.
     */
    ~WaveletOpenmpSep();
    /**
    * Transcode frame.
    * @param bgra_frame frame in BGRA format to transcode
    * @param out optional frame process timers
    * @param error optional string to write error
    */
    bool getFrame(uchar4 *bgra_frame, det_output *out = NULL);
	private:
    /**
     * Delete all buffers and set to initial state.
     */
    void deleteResources();
    void deleteBuffers();
    /**
     * Try to create buffers and set process state to copy buffers.
     * @param error optional string to write error
     * @return success of create kernel buffers
     */
    bool createResources();
    bool createBuffers();
    bool createTimers();

    void transformHor(float *out_image, float *in_image, size_t width, size_t height, double &time);
    void transformVert(float *out_image, float *in_image, size_t width, size_t height, double &time);

    float *temp_image_1;
    float *temp_image_2;

    WaveletOpenmpParamSep *param;
};

#endif

