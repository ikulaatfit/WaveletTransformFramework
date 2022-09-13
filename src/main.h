#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <vector>
#include <CL/cl.h>

typedef std::vector<float> v_float;
typedef std::vector<double> v_double;
typedef std::vector<int> v_int;
typedef std::vector<unsigned int> v_uint;

typedef std::vector<cl_float> v_cl_float;
typedef std::vector<cl_int> v_cl_int;
typedef std::vector<cl_uint> v_cl_uint;

typedef std::vector<cl_ushort2> v_cl_ushort2;

//#define CLASSIFIER_FROM_HEADER "detector_data"

/**
 * Check validity types
 */
typedef enum e_det_func
  {
    DET_FUNC_UNSET,
    DET_FUNC_ENABLED,
    DET_FUNC_DISABLED
  }det_func;

typedef enum e_ocl_state
{
  OCL_STATE_CONTEXT,
  OCL_STATE_QUEUE,
  OCL_STATE_PROGRAM,
  OCL_STATE_KERNELS,
  OCL_STATE_BUFFERS,
  OCL_STATE_ALL
}ocl_state;

#define DET_UNDEF               -1 ///< undefined value

#define DET_PROGRAM_ID_GPU_DATA                0 ///< gpu type preprocessing/postprocessing/position preparing program
#define DET_PROGRAM_ID_GPU_PROC                1 ///< gpu type waldboost processing program
#define PI  	                      3.1415926535f

size_t ceil_align(size_t num, size_t block);
size_t ceil_div(size_t num, size_t block);
double get_time(cl_event proc_event);
void print_device_info(cl_uint device_num, cl_device_id id, FILE *file = stderr);

#endif

