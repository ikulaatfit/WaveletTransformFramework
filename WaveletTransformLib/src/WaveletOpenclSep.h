#ifndef WAVELET_OPENCL_SEP_H
#define WAVELET_OPENCL_SEP_H

#include <string>
#include <climits>
#include <cmath>
#include <omp.h>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <CL/cl.h>

#include "Debug.h"
#include "WaveletOpencl.h"
#include "WaveletOpenclParamSep.h"

class WaveletOpenclSep : public WaveletOpencl
{
	public:
    WaveletOpenclSep(rect image_size, WaveletOpenclParamSep *param);
    ~WaveletOpenclSep();

    bool getFrame(uchar4 *bgra_frame, det_output *out = NULL);
	private:
    void getProfileData(det_output *out);

    void deleteResources();
    void deleteKernels();
    void deleteEvents();

    bool createResources();
    bool createKernels();
    bool createEvents();

    bool transformHor(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_uint act_depth, cl_event *proc_event = NULL);
    bool transformVert(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_uint act_depth, cl_event *proc_event = NULL);

    cl_kernel transform_hor_kernel = NULL;
    cl_kernel transform_vert_kernel = NULL;
    WaveletOpenclParamSep *param;
};

#endif