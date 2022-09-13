#include "WaveletOpenclComb.h"

WaveletOpenclComb::WaveletOpenclComb(rect image_size, WaveletOpenclParamComb *param) : WaveletOpencl(image_size, param)
{
  this->param = param;
  valid = (this->isValid() && this->createResources());
}

WaveletOpenclComb::~WaveletOpenclComb()
{
  this->deleteResources();
}

void WaveletOpenclComb::deleteKernels()
  {
    if(this->transform_kernel != NULL) clReleaseKernel(this->transform_kernel);
    this->transform_kernel = NULL;
  }

bool WaveletOpenclComb::createKernels()
  {
    cl_int err_num;
    this->transform_kernel = clCreateKernel(this->program, "wavelet_block", &err_num);
    if(err_num != CL_SUCCESS){this->error_msg << "Error: clCreateKernel " << this->param->comb_kernel << "\n"; return false;}
    return true;
  }

bool WaveletOpenclComb::createEvents()
{
  this->ev_proc.resize(this->depth * 1);
  for(size_t i = 0; i < this->ev_proc.size(); i++)
    {
      this->ev_proc[i] = clCreateUserEvent(this->context, NULL);
    }
  return true;
}

bool WaveletOpenclComb::createResources()
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

void WaveletOpenclComb::deleteResources()
{
  this->deleteKernels();
  this->deleteEvents();
}

void WaveletOpenclComb::deleteEvents()
{
  for(size_t i = 0; i < this->ev_proc.size(); i++)
    {
      clReleaseEvent(this->ev_proc[i]);
    }
  this->ev_proc.clear();
}

void WaveletOpenclComb::getProfileData(det_output *out)
{
  WaveletOpencl::getProfileData(out);
  if(out == NULL) return;
  for(size_t i = 0; i < this->ev_proc.size(); i++)
    {
      out->proc.push_back(getEventTime(this->ev_proc[i]));
    }
}

bool WaveletOpenclComb::getFrame(uchar4 *bgra_frame, det_output *out)
	{
		if(!this->getFramePre(bgra_frame)) return false;
    cl_mem in_buffer, out_buffer;
    for(cl_uint act_depth = 0; act_depth < this->depth; act_depth++)
	    {
        in_buffer = (act_depth % 2 == 0) ? this->d_scaled_image_2: this->d_scaled_image_1;
        out_buffer = (act_depth % 2 == 0) ? this->d_scaled_image_1 : this->d_scaled_image_2;
        if(!this->transform(out_buffer, in_buffer, (cl_uint)this->param->scaled_image_size.w, (cl_uint)this->param->scaled_image_size.h, act_depth, &(this->ev_proc[act_depth]))) return false;
      }
    in_buffer = (this->depth % 2 == 0) ? this->d_scaled_image_2 : this->d_scaled_image_1;
    out_buffer = (this->depth % 2 == 0) ? this->d_scaled_image_1 : this->d_scaled_image_2;
    if(!this->getFramePost(bgra_frame, in_buffer, out)) return false;
    this->getProfileData(out);
    return true;
	}

bool WaveletOpenclComb::transform(cl_mem out_image, cl_mem in_image, cl_uint width, cl_uint height, cl_uint act_depth, cl_event *proc_event)
{
    cl_int err_msg;
    if(this->param->memless_exec)
      {
        cl_float4 in_image_memless = { 1.0f, 3.0f, 5.0f, 7.0f };
        clSetKernelArg(this->transform_kernel, 0, sizeof(cl_float4), &in_image_memless);
        clSetKernelArg(this->transform_kernel, 1, sizeof(cl_mem), &out_image);
        clSetKernelArg(this->transform_kernel, 2, sizeof(cl_uint), &width);
        clSetKernelArg(this->transform_kernel, 3, sizeof(cl_uint), &(this->param->repeat_count));
      }
    else
      {
        clSetKernelArg(this->transform_kernel, 0, sizeof(cl_mem), &out_image);
        clSetKernelArg(this->transform_kernel, 1, sizeof(cl_mem), &in_image);
        clSetKernelArg(this->transform_kernel, 2, sizeof(cl_uint), &width);
        clSetKernelArg(this->transform_kernel, 3, sizeof(cl_uint), &height);
        clSetKernelArg(this->transform_kernel, 4, sizeof(cl_uint), &act_depth);
      }
    unsigned int act_width = width >> act_depth;
    unsigned int act_height = height >> act_depth;
    if((this->param->comb_sizes.x * this->param->pairs_per_thread.x * 4 > act_width) || (act_width % 2 != 0) ||
       (this->param->comb_sizes.y * this->param->pairs_per_thread.y * 4 > act_height) || (act_height % 2 != 0))
      {
        error_msg << "Error: transform: Wrong parameters for actual image size.\n";
        return false;
      }
    proc_dim local = this->param->comb_sizes;
    proc_dim global;
    unsigned int borders = 2 * (this->param->wavelet_info->width * 2 - 1) * this->param->wavelet_info->steps;
    if(this->param->comb_kernel.find("_in") != std::string::npos)
      {
        //global = proc_dim(ceil_align(act_width * local.x / (2 * (local.x - 4)), local.x), ceil_align(act_height * local.y / (2 * (local.y - 4)), local.y));
        //global = proc_dim(ceil_align(act_width * local.x / (2 * (local.x * param->hor_pairs - borders)), local.x), ceil_align(act_height * local.y / (2 * (local.y * param->vert_pairs - borders)), local.y));
        global = proc_dim(ceilDivBy((size_t)act_width, (2 * (local.x * param->pairs_per_thread.x - borders))), ceilDivBy((size_t)act_height, (2 * (local.y * param->pairs_per_thread.y - borders)))) * local;
      }
    else
      {
        global = proc_dim(alignTo((size_t)act_width, 2 * this->param->pairs_per_thread.x), alignTo((size_t)act_height, 2 * this->param->pairs_per_thread.y)) * local;
      }
    
    if((err_msg = clEnqueueNDRangeKernel(this->queue, this->transform_kernel, 2, NULL, (size_t *)&global, (size_t *)&local, 0, NULL, proc_event)) != CL_SUCCESS){error_msg << "Error: clEnqueueNDRangeKernel transform_comb_kernel " << err_msg << "\n"; return false;};
#ifdef DEBUG_CL
    if((err_msg = clFinish(this->queue)) != CL_SUCCESS){ error_msg << "Error: clFinish->clEnqueueNDRangeKernel->transform " << err_msg << "\n"; return false; }
#endif
    return true;
}