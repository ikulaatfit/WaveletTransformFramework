#ifndef WAVELET_OPENCL_COMB_H
#define WAVELET_OPENCL_COMB_H

#include <string>
#include <climits>
#include <cmath>
#include <omp.h>

#include "Debug.h"
#include "WaveletOpencl.h"
#include "WaveletOpenclParamComb.h"

class WaveletOpenclComb : public WaveletOpencl
{
	public:
    WaveletOpenclComb(rect image_size, WaveletOpenclParamComb *param);
    ~WaveletOpenclComb();

    bool getFrame(uchar4 *bgra_frame, det_output *out = NULL);
	private:
    void getProfileData(det_output *out);

    void deleteResources();
    void deleteKernels();
    void deleteEvents();

    bool createResources();
    bool createKernels();
    bool createEvents();

    bool transform(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_uint act_depth, cl_event *proc_event = NULL);

    cl_kernel transform_kernel = NULL;
    WaveletOpenclParamComb *param;
};

#endif