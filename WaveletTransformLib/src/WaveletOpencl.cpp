#include "WaveletOpencl.h"

WaveletOpencl::WaveletOpencl(rect image_size, WaveletOpenclParam *param) : Wavelet(image_size, param)
{
  this->param = param;
  this->valid = (this->selectDevice(this->param->dev_type, this->param->dev_id) && this->createResources());
  #if DEBUG_LEVEL == DEBUG_ALL
    this->printDebug();
  #endif
}

WaveletOpencl::~WaveletOpencl()
{
  this->deleteResources();
}

bool WaveletOpencl::isValid()
{
  return this->valid;
}

void WaveletOpencl::deleteKernels()
  {
    if(this->rgba_gray_kernel != NULL) clReleaseKernel(this->rgba_gray_kernel);
    this->rgba_gray_kernel = NULL;

    if(this->gray_rgba_kernel != NULL) clReleaseKernel(this->gray_rgba_kernel);
    this->gray_rgba_kernel = NULL;

    if(this->resize_image_kernel != NULL) clReleaseKernel(this->resize_image_kernel);
    this->gray_rgba_kernel = NULL;

    if(this->program != NULL) clReleaseProgram(this->program);
    this->program = NULL;
  }

void WaveletOpencl::deleteQueue()
  {
    if(this->queue != NULL) clReleaseCommandQueue(this->queue);
    this->queue = NULL;

    if(this->context != NULL) clReleaseContext(this->context);
    this->context = NULL;
  }

void WaveletOpencl::deleteResources()
  {
    this->deleteEvents();
    this->deleteKernels();
    this->deleteBuffers();
    this->deleteQueue();
  }

void WaveletOpencl::deleteBuffers()
{
    if(this->d_orig_image_1 != NULL) clReleaseMemObject(this->d_orig_image_1);
    this->d_orig_image_1 = NULL;

    if(this->d_orig_image_2 != NULL) clReleaseMemObject(this->d_orig_image_2);
    this->d_orig_image_2 = NULL;

    if(this->d_scaled_image_1 != NULL) clReleaseMemObject(this->d_scaled_image_1);
    this->d_scaled_image_1 = NULL;

    if(this->d_scaled_image_2 != NULL) clReleaseMemObject(this->d_scaled_image_2);
    this->d_scaled_image_2 = NULL;
}

bool WaveletOpencl::selectDevice(cl_device_type dev_type, cl_uint device_num)
  {
    cl_uint platform_count;
    cl_uint device_count;
    cl_int act_device_num = 0;
    cl_uint compute_units_count;
    cl_device_id subdevice;
    cl_int err_msg;
    if(clGetPlatformIDs(0, NULL, &platform_count) != CL_SUCCESS){this->error_msg << "Error: clGetPlatformIDs\n"; return false;}
    cl_platform_id *platform_ids = new cl_platform_id[platform_count];
    if(clGetPlatformIDs(platform_count, platform_ids, NULL) != CL_SUCCESS){this->error_msg << "Error: clGetPlatformIDs\n"; return false;}
    for(unsigned int i = 0; i < platform_count; i++)
      {
        cl_int temp = clGetDeviceIDs(platform_ids[i], dev_type, 0, NULL, &device_count);
        if(clGetDeviceIDs(platform_ids[i], dev_type, 0, NULL, &device_count) != CL_SUCCESS) continue;
        if (act_device_num + device_count <= device_num)
          {
            act_device_num += device_count;
            continue;
          }
        cl_device_id *device_ids = new cl_device_id[device_num - act_device_num + 1];
        if (clGetDeviceIDs(platform_ids[i], dev_type, device_num - act_device_num + 1, device_ids, NULL) != CL_SUCCESS) continue;
        this->device = device_ids[device_num - act_device_num];
        delete [] device_ids;
        delete [] platform_ids;
		//print_device_info(act_device_num, this->device);
		oclPrintDeviceInfo(this->device);
        if(clGetDeviceInfo(this->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), (void *)&compute_units_count, NULL) != CL_SUCCESS) return false;
        // get subdevices
        size_t device_version_size;
        if(clGetDeviceInfo(this->device, CL_DEVICE_VERSION, NULL, NULL, &device_version_size) != CL_SUCCESS) return false;
        char *device_version = (char *)malloc(device_version_size);
        if(clGetDeviceInfo(this->device, CL_DEVICE_VERSION, device_version_size, (void *)device_version, NULL) != CL_SUCCESS) return false;
        free(device_version);
        if(compute_units_count < this->param->subdevice_size)
          {
            this->error_msg << "Info: actual subdevice size " << this->param->subdevice_size << " is higher than maximum device compute units count " << compute_units_count << ". Setting maximum compute units count.\n";
            this->param->subdevice_size = 0;
          }
        if(this->param->subdevice_size == 0) return true;
        size_t partition_property_size;
        if(clGetDeviceInfo(this->device, CL_DEVICE_PARTITION_PROPERTIES, 0, NULL, &partition_property_size) != CL_SUCCESS) return false;
        if(partition_property_size == 0) { this->error_msg << "Warning: clCreateSubdevices is not supported by device."; return true; }
        cl_device_partition_property *partition_property = (cl_device_partition_property *)malloc(partition_property_size);
        if(clGetDeviceInfo(this->device, CL_DEVICE_PARTITION_PROPERTIES, partition_property_size, partition_property, NULL) != CL_SUCCESS) return false;
        for(int i = 0; i < partition_property_size / sizeof(cl_device_partition_property); i++)
          {
            if(partition_property[i] != CL_DEVICE_PARTITION_BY_COUNTS) continue;
            cl_device_partition_property properties[] = { CL_DEVICE_PARTITION_BY_COUNTS, this->param->subdevice_size, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0 };
            if((err_msg = clCreateSubDevices(this->device, properties, 1, &subdevice, NULL)) != CL_SUCCESS){this->error_msg << "Error: clCreateSubDevices " << err_msg << " \n"; return false;}
            this->device = subdevice;
			//print_device_info(act_device_num, this->device);
			oclPrintDeviceInfo(this->device);
            return true;
          }
        free(partition_property);
        this->error_msg << "Warning: clCreateSubdevices is not supported by device.";
        return true;
      }
    this->error_msg << "Error: No device found\n";
    return false;
  }

void WaveletOpencl::printDevicesInfo(std::ostream *error_msg, FILE *file)
  {
    cl_uint platform_count;
    cl_uint device_count;
    cl_uint act_device_num = 0;
    if (clGetPlatformIDs(0, NULL, &platform_count) != CL_SUCCESS){ if(error_msg != NULL) *error_msg << "Error: clGetPlatformIDs\n"; return; }
    cl_platform_id *platform_ids = new cl_platform_id[platform_count];
    if(clGetPlatformIDs(platform_count, platform_ids, NULL) != CL_SUCCESS){ if(error_msg != NULL) *error_msg << "Error: clGetPlatformIDs\n"; return; }
    for (unsigned int i = 0; i < platform_count; i++)
      {
        if(clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count) != CL_SUCCESS) continue;
        if(device_count == 0) continue;
        cl_device_id *device_ids = new cl_device_id[device_count];
        if(clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, device_count, device_ids, NULL) != CL_SUCCESS) continue;
        for(unsigned int i = 0; i < device_count; i++)
          {
            //print_device_info(act_device_num, device_ids[i], file);
			oclPrintDeviceInfo(device_ids[i]);
            act_device_num++;
          }
        delete [] device_ids;
      }
    delete [] platform_ids;
  }

bool WaveletOpencl::createKernels()
  {
    cl_int err_num;
    std::string kernel_filename("kernels/gpu.cl");
    if((ocl_compile = compileOpenclSource(this->program, this->context, this->device, this->param->createBuildParam(), kernel_filename, &this->error_msg)) == OCL_COMPILE_FAILED){this->error_msg << "Error: Compilation of binary failed\n"; return false;}
#ifdef DEBUG_CL
    size_t program_size;
    clGetProgramBuildInfo(this->program, this->device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &program_size);
    char *data = new char[program_size];
    clGetProgramBuildInfo(this->program, this->device, CL_PROGRAM_BUILD_LOG, program_size, data, NULL);
    this->error_msg << "Build log:\n" << data << "\n";
#endif
    
    if(this->param->memless_exec || this->param->benchmark_proc) return true;

    this->rgba_gray_kernel = clCreateKernel(this->program, "conv_rgba_gray", &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateKernel conv_rgba_gray\n"; return false;}
    this->gray_rgba_kernel = clCreateKernel(this->program, "conv_gray_rgba", &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateKernel conv_gray_rgba\n"; return false;}
    this->resize_image_kernel = clCreateKernel(this->program, "resize_image", &err_num);
    if(err_num != CL_SUCCESS){ this->error_msg << "Error: clCreateKernel resize_image\n"; return false; }
    return true;
  }

bool WaveletOpencl::createQueue()
  {
    cl_int err_num;
    this->context = clCreateContext(NULL, 1, &this->device, NULL, NULL, &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateContext\n"; return false;}
    this->queue = clCreateCommandQueue(this->context, this->device, CL_QUEUE_PROFILING_ENABLE, &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateCommandQueue\n"; return false;}
    return true;
  }

bool WaveletOpencl::createResources()
{
  if(!this->image_size.isValid() || 
     !this->createQueue() ||
     !this->createKernels() ||
     !this->createBuffers() ||
     !this->createEvents())
    {
      this->deleteResources();
      return false;
    }
  return true;
}

cl_mem WaveletOpencl::createMemObject(rect image_size, size_t element_size, cl_image_format image_format, cl_int *err_num)
{
  if(this->param->image_mem_type == OPENCL_MEM_TYPE_GLOBAL)
    {
      return clCreateBuffer(this->context, CL_MEM_READ_WRITE, image_size.getSize() * element_size, NULL, err_num);
    }
  else
    {
      return clCreateImage2D(this->context, CL_MEM_READ_WRITE, &image_format, image_size.w, image_size.h, 0, NULL, err_num);
    }
}

cl_int WaveletOpencl::writeMemObject(cl_mem out_image, void *in_image, rect image_size, size_t element_size, cl_event *copy_event)
{
  if(this->param->image_mem_type == OPENCL_MEM_TYPE_GLOBAL)
  {
    return clEnqueueWriteBuffer(this->queue, out_image, CL_FALSE, 0, image_size.getSize() * element_size, in_image, 0, NULL, copy_event);
  }
  else
  {
    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { image_size.x, image_size.y, 1 };
    return clEnqueueWriteImage(this->queue, out_image, CL_FALSE, origin, region, 0, 0, in_image, 0, NULL, copy_event);
  }
}

cl_int WaveletOpencl::readMemObject(void *out_image, cl_mem in_image, rect image_size, size_t element_size, cl_event *copy_event)
{
  if(this->param->image_mem_type == OPENCL_MEM_TYPE_GLOBAL)
  {
    return clEnqueueReadBuffer(this->queue, in_image, CL_FALSE, 0, image_size.getSize() * element_size, (void *)out_image, 0, NULL, copy_event);
  }
  else
  {
    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { image_size.x, image_size.y, 1 };
    return clEnqueueReadImage(this->queue, in_image, CL_FALSE, origin, region, 0, 0, out_image, 0, NULL, copy_event);
  }
}

bool WaveletOpencl::createBuffers()
{
  cl_int err_num;
  if((d_scaled_image_1 = createMemObject(this->param->scaled_image_size, sizeof(cl_float), { CL_R, CL_FLOAT }, &err_num)) == NULL)
    {this->error_msg << "Error: clCreateBuffer d_scaled_image_1\n"; return false;}
  if((d_scaled_image_2 = createMemObject(this->param->scaled_image_size, sizeof(cl_float), { CL_R, CL_FLOAT }, &err_num)) == NULL)
    {this->error_msg << "Error: clCreateBuffer d_scaled_image_2\n"; return false;}

  if(this->param->memless_exec || this->param->benchmark_proc) return true;

  if((d_orig_image_1 = createMemObject(image_size, sizeof(cl_uchar4), { CL_RGBA, CL_UNSIGNED_INT8 }, &err_num)) == NULL)
    {this->error_msg << "Error: clCreateBuffer d_orig_image_1\n"; return false;}
  if(this->param->scaled_image_size != this->image_size)
    {
    if((d_orig_image_2 = createMemObject(image_size, sizeof(cl_float), { CL_R, CL_FLOAT }, &err_num)) == NULL)
        {this->error_msg << "Error: clCreateBuffer d_orig_image_2\n"; return false;}
    }
  return true;
}

bool WaveletOpencl::createEvents()
{
  if(this->param->memless_exec || this->param->benchmark_proc) return true;

  int event_count = (this->param->memless_exec || this->param->benchmark_proc) ? 0 : ((this->param->scaled_image_size == this->image_size) ? 2 : 3);
  this->ev_pre.resize(event_count);
  this->ev_post.resize(event_count);
  for(size_t i = 0; i < this->ev_pre.size(); i++)
    {
      this->ev_pre[i] = clCreateUserEvent(this->context, NULL);
    }
  for(size_t i = 0; i < this->ev_post.size(); i++)
    {
      this->ev_post[i] = clCreateUserEvent(this->context, NULL);
    }
  return true;
}

void WaveletOpencl::deleteEvents()
{
  for(size_t i = 0; i < this->ev_pre.size(); i++)
    {
      clReleaseEvent(this->ev_pre[i]);
    }
  for(size_t i = 0; i < this->ev_post.size(); i++)
    {
      clReleaseEvent(this->ev_post[i]);
    }
  this->ev_pre.clear();
  this->ev_post.clear();
}

bool WaveletOpencl::getFramePre(uchar4 *bgra_frame)
	{
    cl_int err_msg;
		if(!this->isValid())
			{
        this->error_msg << "Error: Detector isnt in valid state.\n";
				return false;
			}
    if(this->param->memless_exec || this->param->benchmark_proc) return true;

    if((err_msg = writeMemObject(this->d_orig_image_1, (void *)bgra_frame, this->image_size, sizeof(cl_uchar4), &(this->ev_pre[0]))) != CL_SUCCESS){ error_msg << "Error: clEnqueueWriteBuffer " << err_msg << "\n"; return false; }
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS) { error_msg << "Error: clFinish clEnqueueWriteBuffer " << err_msg << "\n"; return false; }
#endif
    cl_mem *convertRgbaGray_out = (this->param->scaled_image_size == this->image_size) ? &this->d_scaled_image_2 : &this->d_orig_image_2;
    if(!this->convRgbaGray(*convertRgbaGray_out, this->d_orig_image_1, (cl_uint)this->image_size.w, (cl_uint)this->image_size.h, &(this->ev_pre[1]))) return false;
    if(this->param->scaled_image_size != this->image_size)
      {
        if(!this->resizeImage(this->d_scaled_image_2, this->d_orig_image_2, { (cl_uint)this->param->scaled_image_size.x, (cl_uint)this->param->scaled_image_size.y }, { (cl_uint)this->image_size.x, (cl_uint)this->image_size.y }, &(this->ev_pre[2]))) return false;
      }
    return true;
	}

bool WaveletOpencl::resizeImage(cl_mem out_image, cl_mem in_image, cl_uint2 out_image_size, cl_uint2 in_image_size, cl_event *resize_event = NULL)
{
  cl_int err_msg;
  clSetKernelArg(this->resize_image_kernel, 0, sizeof(cl_mem), &out_image);
  clSetKernelArg(this->resize_image_kernel, 1, sizeof(cl_mem), &in_image);
  clSetKernelArg(this->resize_image_kernel, 2, sizeof(cl_uint2), &out_image_size);
  clSetKernelArg(this->resize_image_kernel, 3, sizeof(cl_uint2), &in_image_size);

  size_t local[2] = { 32, 8 };
  size_t global[2] = { alignTo((size_t)out_image_size.s[0], local[0]), alignTo((size_t)out_image_size.s[1], local[1]) };
  if((err_msg = clEnqueueNDRangeKernel(this->queue, this->resize_image_kernel, 2, NULL, global, local, 0, NULL, resize_event)) != CL_SUCCESS){ error_msg << "Error: clEnqueueNDRangeKernel resize_image_kernel " << err_msg << "\n"; return false; };
#ifdef DEBUG_CL
  if((err_msg = clFinish(this->queue)) != CL_SUCCESS){ error_msg << "Error: clFinish->clEnqueueNDRangeKernel->resizeImage " << err_msg << "\n"; return false; }
#endif
  return true;
}

void WaveletOpencl::getProfileData(det_output *out)
{
  if(out == NULL) return;
  for(size_t i = 0; i < this->ev_pre.size(); i++)
    {
      out->pre.push_back(getEventTime(this->ev_pre[i]));
    }
  for(size_t i = 0; i < this->ev_post.size(); i++)
    {
      out->post.push_back(getEventTime(this->ev_post[i]));
    }
}

bool WaveletOpencl::getFramePost(uchar4 *bgra_frame, cl_mem in_image, det_output *out)
	{
    cl_int err_msg;
		if(!this->isValid())
			{
        this->error_msg << "Error: Detector isnt in valid state.\n";
				return false;
			}
    if((!this->param->memless_exec) && (!this->param->benchmark_proc))
      {
        cl_int post_event_counter = 0;
        if(this->param->scaled_image_size != this->image_size)
          {
            if(!this->resizeImage(this->d_orig_image_2, in_image, { (cl_uint)this->image_size.x, (cl_uint)this->image_size.y }, { (cl_uint)this->param->scaled_image_size.x, (cl_uint)this->param->scaled_image_size.y }, &(this->ev_post[post_event_counter++]))) return false;
          }
        cl_mem *convertRgbaGray_in = (this->param->scaled_image_size == this->image_size) ? &in_image : &this->d_orig_image_2;
        if(!this->convGrayRgba(this->d_orig_image_1, *convertRgbaGray_in, (cl_uint)this->image_size.w, (cl_uint)this->image_size.h, &(this->ev_post[post_event_counter++]))){ return false; }
        if((err_msg = readMemObject((void *)bgra_frame, this->d_orig_image_1, this->image_size, sizeof(cl_uchar4), &(this->ev_post[post_event_counter++]))) != CL_SUCCESS){ error_msg << "Error: clEnqueueReadBuffer " << err_msg << "\n"; return false; }
        //if((err_msg = clEnqueueReadBuffer(this->queue, this->d_orig_image_1, CL_FALSE, 0, this->image_size.getSize() * sizeof(cl_uchar4), (void *)bgra_frame, 0, NULL, &(this->ev_post[post_event_counter++]))) != CL_SUCCESS){ error_msg << "Error: clEnqueueReadBuffer " << err_msg << "\n"; return false; }
      }
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS) { error_msg << "Error: clFinish->clEnqueueReadBuffer " << err_msg << "\n"; return false; }
#else
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS) { error_msg << "Error: clFinish " << err_msg << "\n"; return false; }
#endif
    return true;
	}

bool WaveletOpencl::convRgbaGray(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_event *proc_event)
  {
    cl_int err_msg;
    clSetKernelArg(this->rgba_gray_kernel, 0, sizeof(cl_mem), &out_image);
    clSetKernelArg(this->rgba_gray_kernel, 1, sizeof(cl_mem), &in_image);
    clSetKernelArg(this->rgba_gray_kernel, 2, sizeof(cl_uint), &width);
    clSetKernelArg(this->rgba_gray_kernel, 3, sizeof(cl_uint), &height);
    
    size_t local[2] = {32, 8};
    size_t global[2] = { alignTo((size_t)width, local[0]), alignTo((size_t)height, local[1])};
    if((err_msg = clEnqueueNDRangeKernel(this->queue, this->rgba_gray_kernel, 2, NULL, global, local, 0, NULL, proc_event)) != CL_SUCCESS){error_msg << "Error: clEnqueueNDRangeKernel rgba_gray_kernel " << err_msg << "\n"; return false;};
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS){error_msg << "Error: clFinish->clEnqueueNDRangeKernel->rgbaGray " << err_msg << "\n"; return false;}
#endif
    return true;
  }

bool WaveletOpencl::convGrayRgba(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_event *proc_event)
  {
    cl_int err_msg;
    cl_uint depth = this->depth;
    clSetKernelArg(this->gray_rgba_kernel, 0, sizeof(cl_mem), &out_image);
    clSetKernelArg(this->gray_rgba_kernel, 1, sizeof(cl_mem), &in_image);
    clSetKernelArg(this->gray_rgba_kernel, 2, sizeof(cl_uint), &width);
    clSetKernelArg(this->gray_rgba_kernel, 3, sizeof(cl_uint), &height);
    clSetKernelArg(this->gray_rgba_kernel, 4, sizeof(cl_uint), &depth);
    
    size_t local[2] = {32, 8};
    size_t global[2] = { alignTo((size_t)width, local[0]), alignTo((size_t)height, local[1]) };
    if((err_msg = clEnqueueNDRangeKernel(this->queue, this->gray_rgba_kernel, 2, NULL, global, local, 0, NULL, proc_event)) != CL_SUCCESS){error_msg << "Error: clEnqueueNDRangeKernel gray_rgba_kernel " << err_msg << "\n"; return false;};
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS){ error_msg << "Error: clFinish->clEnqueueNDRangeKernel->grayRgba " << err_msg << "\n"; return false; }
#endif
    return true;
  }

/**
  * Print debug information
  */
void WaveletOpencl::printDebug()
{
  this->Wavelet::printDebug();
    
  std::ostringstream str;
  std::string ocl_compile;
  switch(this->ocl_compile)
    {
      case OCL_COMPILE_FAILED:
        ocl_compile = "failed";
      break;
      case OCL_COMPILE_CACHED:
        ocl_compile = "cached";
      break;
      case OCL_COMPILE_COMPILED:
        ocl_compile = "compiled";
      break;
    }
  fprintf(stderr, "\nCompile information\n" \
                  "  executable: %s\n", ocl_compile.c_str());
}
