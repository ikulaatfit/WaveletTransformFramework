#include "WaveletOpenclSep.h"

WaveletOpenclSep::WaveletOpenclSep(rect image_size, WaveletOpenclParamSep *param) : WaveletOpencl(image_size, param)
{
  this->param = param;
  valid = (this->isValid() && this->createResources());
}

WaveletOpenclSep::~WaveletOpenclSep()
{
  this->deleteResources();
}

void WaveletOpenclSep::deleteKernels()
  {
    if(this->transform_hor_kernel != NULL) clReleaseKernel(this->transform_hor_kernel);
    this->transform_hor_kernel = NULL;

    if(this->transform_vert_kernel != NULL) clReleaseKernel(this->transform_vert_kernel);
    this->transform_vert_kernel = NULL;
  }

bool WaveletOpenclSep::createKernels()
  {
    cl_int err_num;
    this->transform_vert_kernel = clCreateKernel(this->program, "wavelet_vert", &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateKernel " << this->param->vert_kernel << "\n"; return false;}
    this->transform_hor_kernel = clCreateKernel(this->program, "wavelet_hor", &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateKernel " << this->param->hor_kernel << "\n"; return false;}
    return true;
  }

bool WaveletOpenclSep::createEvents()
{
  this->ev_proc.resize(this->depth * 2);
  for(size_t i = 0; i < this->ev_proc.size(); i++)
    {
      this->ev_proc[i] = clCreateUserEvent(this->context, NULL);
    }
  return true;
}

void WaveletOpenclSep::deleteEvents()
{
  for(size_t i = 0; i < this->ev_proc.size(); i++)
    {
      clReleaseEvent(this->ev_proc[i]);
    }
  this->ev_proc.clear();
}

void WaveletOpenclSep::getProfileData(det_output *out)
{
  if(out == NULL) return;
  for(size_t i = 0; i < this->ev_proc.size(); i++)
    {
      out->proc.push_back(getEventTime(this->ev_proc[i]));
    }
}


bool WaveletOpenclSep::createResources()
{
  if(!this->image_size.isValid() ||
     !this->createKernels() ||
     !this->createEvents())
    {
      this->deleteResources();
      return false;
    }
  return true;
}

void WaveletOpenclSep::deleteResources()
  {
    this->deleteKernels();
    this->deleteEvents();
  }


bool WaveletOpenclSep::getFrame(uchar4 *bgra_frame, det_output *out)
	{
    if(!this->getFramePre(bgra_frame)) return false;
    for(cl_uint act_depth = 0; act_depth < this->depth; act_depth++)
      {
        if((!this->transformHor(this->d_scaled_image_1, this->d_scaled_image_2, (cl_uint)this->param->scaled_image_size.w, (cl_uint)this->param->scaled_image_size.h, act_depth, &(this->ev_proc[act_depth * 2]))) ||
           (!this->transformVert(this->d_scaled_image_2, this->d_scaled_image_1, (cl_uint)this->param->scaled_image_size.w, (cl_uint)this->param->scaled_image_size.h, act_depth, &(this->ev_proc[act_depth * 2 + 1]))))
          return false;
      }
    if(!this->getFramePost(bgra_frame, this->d_scaled_image_2, out)) return false;
    return true;
	}

bool WaveletOpenclSep::transformVert(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_uint act_depth, cl_event *proc_event)
{
    cl_int err_msg;
    clSetKernelArg(this->transform_vert_kernel, 0, sizeof(cl_mem), &out_image);
    clSetKernelArg(this->transform_vert_kernel, 1, sizeof(cl_mem), &in_image);
    clSetKernelArg(this->transform_vert_kernel, 2, sizeof(cl_uint), &width);
    clSetKernelArg(this->transform_vert_kernel, 3, sizeof(cl_uint), &height);
    clSetKernelArg(this->transform_vert_kernel, 4, sizeof(cl_uint), &act_depth);
    unsigned int act_width = width >> act_depth;
    unsigned int act_height = height >> act_depth;
    if((this->param->vert_sizes.y * this->param->pairs_per_thread.y * 4 > act_height) || ((act_height % (this->param->vert_sizes.y * this->param->pairs_per_thread.y * 4)) != 0) ||
        (this->param->vert_sizes.x > act_width) || ((act_width % this->param->vert_sizes.x) != 0))
      {
        error_msg << "Error: Wrong parameters for actual image size.\n";
        return false;
      }
    proc_dim local = this->param->vert_sizes;
    proc_dim global;
    if((this->param->vert_kernel.find("split") != std::string::npos) || (this->param->vert_kernel.find("line") != std::string::npos))
      {
        global = proc_dim(act_width, act_height/(this->param->pairs_per_thread.y * 2));
      }
    else
      {
        global = proc_dim(act_width);
      }
    global.alignTo(local);
    if((err_msg = clEnqueueNDRangeKernel(this->queue, this->transform_vert_kernel, 2, NULL, (size_t *)&global, (size_t *)&local, 0, NULL, proc_event)) != CL_SUCCESS){error_msg << "Error: clEnqueueNDRangeKernel transform_vert_kernel " << err_msg << "\n"; return false;};
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS){ error_msg << "Error: clFinish->clEnqueueNDRangeKernel->transformVert " << err_msg << "\n"; return false; }
#endif
    return true;
}

bool WaveletOpenclSep::transformHor(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_uint act_depth, cl_event *proc_event)
{
    cl_int err_msg;
    clSetKernelArg(this->transform_hor_kernel, 0, sizeof(cl_mem), &out_image);
    clSetKernelArg(this->transform_hor_kernel, 1, sizeof(cl_mem), &in_image);
    clSetKernelArg(this->transform_hor_kernel, 2, sizeof(cl_uint), &width);
    clSetKernelArg(this->transform_hor_kernel, 3, sizeof(cl_uint), &height);
    clSetKernelArg(this->transform_hor_kernel, 4, sizeof(cl_uint), &act_depth);
    unsigned int act_width = width >> act_depth;
    unsigned int act_height = height >> act_depth;
    if((this->param->hor_sizes.x * this->param->pairs_per_thread.x * 2 > act_width) || ((act_width % (this->param->hor_sizes.x * this->param->pairs_per_thread.x * 2)) != 0) ||
        (this->param->hor_sizes.y > act_height) || ((act_height % this->param->hor_sizes.y) != 0))
      {
        error_msg << "Error: Wrong parameters for actual image size.\n";
        return false;
      }
    proc_dim local = this->param->hor_sizes;
    proc_dim global;
    if(this->param->hor_kernel.find("split") != std::string::npos)
      {
        global = proc_dim((act_width/(this->param->pairs_per_thread.x * 2)),act_height);
      }
    else
      {
        global = proc_dim(1,act_height);
      }
    global.alignTo(local);
    if((err_msg = clEnqueueNDRangeKernel(this->queue, this->transform_hor_kernel, 2, NULL, (size_t *)&global, (size_t *)&local, 0, NULL, proc_event)) != CL_SUCCESS){error_msg << "Error: clEnqueueNDRangeKernel transform_hor_kernel " << err_msg << "\n"; return false;}
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS){ error_msg << "Error: clFinish->clEnqueueNDRangeKernel->transformHor " << err_msg << "\n"; return false; }
#endif
    return true;
}
