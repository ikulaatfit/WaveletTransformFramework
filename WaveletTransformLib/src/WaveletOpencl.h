#ifndef WAVELET_OPENCL_H
#define WAVELET_OPENCL_H

#include <string>
#include <ostream>
#include <climits>
#include <cmath>
#include <omp.h>

#include "Debug.h"
#include "Wavelet.h"
#include "WaveletOpenclParam.h"
#include <CL/cl.h>
#include <OclHelper.h>

typedef std::vector<cl_event> v_cl_event;

class WaveletOpencl : public Wavelet
{
	public:
    virtual ~WaveletOpencl();
    
    virtual bool isValid();
    virtual bool getFrame(uchar4 *bgra_frame, det_output *out = NULL) = 0;
    static void printDevicesInfo(std::ostream *error_msg = NULL, FILE *file = stderr);
  protected:
    WaveletOpencl(rect image_size, WaveletOpenclParam *param);
    bool getFramePre(uchar4 *bgra_frame);
    bool getFramePost(uchar4 *bgra_frame, cl_mem in_image, det_output *out = NULL);
    void getProfileData(det_output *out);

    void deleteResources();
    void deleteBuffers();
    void deleteQueue();
    void deleteKernels();
    void deleteEvents();

    bool selectDevice(cl_device_type dev_type, cl_uint device_id = 0);

    bool createResources();
    bool createQueue();
    bool createBuffers();
    bool createKernels();
    bool createEvents();

    bool convRgbaGray(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_event *proc_event = NULL);
    bool convGrayRgba(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_event *proc_event = NULL);

    bool resizeImage(cl_mem out_image, cl_mem in_image, cl_uint2 out_image_size, cl_uint2 in_image_size, cl_event *resize_event);

    cl_mem createMemObject(rect image_size, size_t element_size, cl_image_format image_format, cl_int *err_num);
    cl_int writeMemObject(cl_mem out_image, void *in_image, rect image_size, size_t element_size, cl_event *copy_event);
    cl_int readMemObject(void *out_image, cl_mem in_image, rect image_size, size_t element_size, cl_event *copy_event);

    cl_mem d_orig_image_1 = NULL;
    cl_mem d_orig_image_2 = NULL;
    cl_mem d_scaled_image_1 = NULL;
    cl_mem d_scaled_image_2 = NULL;

    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel rgba_gray_kernel = NULL;
    cl_kernel gray_rgba_kernel = NULL;
    cl_kernel resize_image_kernel = NULL;
    cl_device_id device = NULL;
    v_cl_event ev_proc;

    bool valid = false;
    WaveletOpenclParam *param;
    e_ocl_compile ocl_compile;
    void printDebug();
  private:
    v_cl_event ev_pre;
    v_cl_event ev_post;
};

#endif
