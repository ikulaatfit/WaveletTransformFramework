#include "WaveletOpenclParam.h"

WaveletOpenclParam::WaveletOpenclParam(engine_type in_engine) : WaveletParam(in_engine)
  {
    this->clear();
  }

void WaveletOpenclParam::clear()
  {
    this->pairs_per_thread = proc_dim(1, 1);
    this->dev_type = CL_DEVICE_TYPE_GPU;
    this->dev_id = 0;
    this->optim_warp = WAVELET_OPTIM_WARP_NONE;
    this->warp_size = 32;
    this->vert_proc = WAVELET_PROC_TYPE_BLAZ_NORMAL;
    this->hor_proc = WAVELET_PROC_TYPE_BLAZ_NORMAL;
    this->subdevice_size = 0;
    this->image_mem_type = OPENCL_MEM_TYPE_GLOBAL;
    this->double_buffering = true;
    this->memless_exec = false;
    this->repeat_count = 1;
    this->benchmark_proc = false;
    this->gen_filter_body = true;
  }

std::string WaveletOpenclParam::createBuildParam()
  {
    std::ostringstream str;
    if(this->interlaced) str << " -D WAVELET_OUTPUT_INTERLACED=1";
    if(this->double_buffering) str << " -D DOUBLE_BUFFERING=1";
    if(this->memless_exec) str << " -D MEMLESS_EXEC=1";
    if(this->gen_filter_body) str << " -D GEN_FILTER_BODY=1";
    str << " -D REPEAT_COUNT=" << this->repeat_count 
        << " -D HOR_PAIRS_PER_THREAD=" << this->pairs_per_thread.x
        << " -D VERT_PAIRS_PER_THREAD=" << this->pairs_per_thread.y
        << " -D COMB_PAIRS_PER_THREAD_X=" << this->pairs_per_thread.x
        << " -D COMB_PAIRS_PER_THREAD_Y=" << this->pairs_per_thread.y
        << " -D WARP_SIZE=" << this->warp_size
        << " -D HOR_PROC_TYPE=" << this->hor_proc
        << " -D VERT_PROC_TYPE=" << this->vert_proc
        << " -D OPTIM_WARP=" << this->optim_warp
        << " -D DEVICE_TYPE=" << ((this->dev_type == CL_DEVICE_TYPE_GPU) ? OPENCL_DEV_TYPE_CPU : OPENCL_DEV_TYPE_GPU)
        << " -D IMAGE_MEM_TYPE=" << this->image_mem_type
        << " -D " << this->wavelet_info->define
		<< " -D FILTER_LENGTH=" << this->wavelet_info->width
		<< " -D LIFTING_STEPS_COUNT=" << this->wavelet_info->steps
        //<< " -cl-nv-verbose"
        << " -I kernels";
    
    #if DEBUG_LEVEL == DEBUG_ALL
      this->printDebug();
    #endif
    return str.str();
  }
/**
  * Print debug information
  */
void WaveletOpenclParam::printDebug()
  {
    this->WaveletParam::printDebug();
    
    std::ostringstream str;
    std::string vert_proc_type, hor_proc_type;
    std::string device_type;
    std::string image_mem_type;
    std::string optim_warp;
    switch(this->optim_warp)
      {
        case WAVELET_OPTIM_WARP_NONE:
          optim_warp = "none";
        break;
        case WAVELET_OPTIM_WARP_LOCAL:
          optim_warp = "local";
        break;
        case WAVELET_OPTIM_WARP_SHUFFLE:
          optim_warp = "shuffle";
        break;
      }
    switch(this->hor_proc)
      {
        case WAVELET_PROC_TYPE_BLAZ_NORMAL:
          hor_proc_type = "blazewicz normal";
        break;
        case WAVELET_PROC_TYPE_BLAZ_REGISTER:
          hor_proc_type = "blazewicz register";
        break;
        case WAVELET_PROC_TYPE_LAAN:
          hor_proc_type = "laan";
        break;
      }
    switch(this->vert_proc)
      {
        case WAVELET_PROC_TYPE_BLAZ_NORMAL:
          vert_proc_type = "blazewicz normal";
        break;
        case WAVELET_PROC_TYPE_BLAZ_REGISTER:
          vert_proc_type = "blazewicz register";
        break;
        case WAVELET_PROC_TYPE_LAAN:
          vert_proc_type = "laan";
        break;
      }
    switch(this->dev_type)
      {
        case CL_DEVICE_TYPE_CPU:
          device_type = "cpu";
        break;
        case CL_DEVICE_TYPE_GPU:
          device_type = "gpu";
        break;
        default:
          device_type = "other";
        break;
      }
      switch(this->image_mem_type)
      {
        case OPENCL_MEM_TYPE_GLOBAL:
          image_mem_type = "global";
        break;
        case OPENCL_MEM_TYPE_TEXTURE:
          image_mem_type = "texture";
        break;
      }
    fprintf(stderr, " warp size:               %d\n"
                    " warp optimalization:     %s\n"
                    " device type:             %s\n"
                    " hor pairs per thread:    %d\n"
                    " hor proc type:           %s\n"
                    " vert pairs per thread:   %d\n"
                    " vert proc type:          %s\n"
                    " image mem type:          %s\n"
                    " double buffering:        %s\n"
                    " repeat count:            %d\n"
                    " memoryless execution:    %s\n", this->warp_size, optim_warp.c_str(), device_type.c_str(), (int)this->pairs_per_thread.x, hor_proc_type.c_str(), (int)this->pairs_per_thread.y, vert_proc_type.c_str(), image_mem_type.c_str(), (this->double_buffering) ? "1" : "0", this->repeat_count, (this->memless_exec) ? "1" : "0");
  }