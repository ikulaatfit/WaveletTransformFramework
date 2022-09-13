#include "WaveletOpenmp.h"

WaveletOpenmp::WaveletOpenmp(rect image_size, WaveletOpenmpParam *param) : Wavelet(image_size, param)
{
  this->param = param;
  this->f_gray_image_1 = NULL;
  this->f_gray_image_2 = NULL;
  omp_set_num_threads(4);
  this->createResources();
}

WaveletOpenmp::~WaveletOpenmp()
{
  this->deleteResources();
}

bool WaveletOpenmp::isValid()
{
  return this->image_size.isValid();
}

void WaveletOpenmp::deleteBuffers()
{
    if(!this->isValid()) return;
    delete [] f_gray_image_1;
    delete [] f_gray_image_2;
}

void WaveletOpenmp::deleteTimers()
{
    if(!this->isValid()) return;
    this->pre_time.clear();
    this->proc_time.clear();
    this->post_time.clear();
}

void WaveletOpenmp::deleteResources()
{
  this->deleteTimers();
  this->deleteBuffers();
}

bool WaveletOpenmp::createBuffers()
{
  if(!this->isValid())
    {
      this->error_msg << "Error: Detector isnt in valid state.\n";
      return false;
    }
  f_gray_image_1 = new float[image_size.getSize()];
  f_gray_image_2 = new float[image_size.getSize()];
  return true;
}

bool WaveletOpenmp::createTimers()
{
  if(!this->isValid())
    {
      this->error_msg << "Error: Detector isnt in valid state.\n";
      return false;
    }
  this->pre_time.resize(1);
  this->post_time.resize(1);
  return true;
}

bool WaveletOpenmp::createResources()
{
  if(!this->image_size.isValid() || 
     !this->createBuffers() ||
     !this->createTimers())
    {
      this->deleteResources();
      return false;
    }
  #if DEBUG_LEVEL == DEBUG_ALL
  this->param->printDebug();
  #endif
  return true;
}

bool WaveletOpenmp::getFramePre(uchar4 *bgra_frame)
{
  if(!this->isValid())
			{
        this->error_msg << "Error: Detector isnt in valid state.\n";
				return false;
			}
    
    this->conv_c_rgba_f_gray(this->f_gray_image_1, bgra_frame, this->image_size.w, this->image_size.h, this->pre_time[0]);
    return true;
}

bool WaveletOpenmp::getFramePost(uchar4 *bgra_frame, float *f_gray_out, float *f_gray_in, det_output *out)
{
  if(!this->isValid())
			{
        this->error_msg << "Error: Detector isnt in valid state.\n";
				return false;
			}
    
    this->conv_f_gray_c_rgba(bgra_frame, this->f_gray_image_1, this->image_size.w, this->image_size.h, this->post_time[0]);
    
    if(out != NULL)
      {
        for(int i = 0; i < this->pre_time.size(); i++)
          {
            out->pre.push_back(this->pre_time[i]);
          }
        for(int i = 0; i < this->proc_time.size(); i++)
          {
            out->proc.push_back(this->proc_time[i]);
          }
        for(int i = 0; i < this->post_time.size(); i++)
          {
            out->post.push_back(this->post_time[i]);
          }
      }
    return true;
}

void WaveletOpenmp::conv_c_rgba_f_gray(float *out_image, uchar4 *in_image, size_t width, size_t height, double &time)
  {
    double start = omp_get_wtime();
    #pragma omp parallel for default(none) shared(width, in_image, out_image, height)
    for(int j = 0; j < height; j++)
      {
        for(size_t i = 0; i < width; i++)
          {
            out_image[j * width + i] = 0.299f * in_image[j * width + i].z + 0.587f * in_image[j * width + i].y + 0.114f * in_image[j * width + i].x;
            //out_image[j * width + i] = in_image[j * width + i].x;
          }
      }
    double end = omp_get_wtime();
    time = (end - start) * 1000.0;
  }


/*void WaveletOpenmp::conv_c_rgba_f_gray(float *out_image, uchar4 *in_image, size_t width, size_t height, float &time)
  {
    double start = omp_get_wtime();
    #pragma omp parallel for default(none) shared(width, in_image, out_image, height)
    for(int j = 0; j < height; j++)
      {
        for(size_t i = 0; i < width; i++)
          {
            out_image[j * width + i] = 0.299f * in_image[j * width + i].z + 0.587f * in_image[j * width + i].y + 0.114f * in_image[j * width + i].x;
          }
      }
    double end = omp_get_wtime();
    time = (end - start) * 1000.0;
  }*/

/*void WaveletOpenmp::conv_f_gray_c_rgba(uchar4 *out_image, float *in_image, size_t width, size_t height, float &time)
  {
    size_t half_width = width / 2;
    size_t half_height = height / 2;
    double start = omp_get_wtime();
    #pragma omp parallel for default(none) shared(width, in_image, out_image, height, half_width, half_height)
    for(int j = 0; j < height; j++)
      {
        for(size_t i = 0; i < width; i++)
          {
			      float temp_val;
			      if((i < half_width) && (j < half_height))
			        {
				        temp_val = abs(in_image[j * width + i]);
			        }
			      else
			        {
				        temp_val = log2(abs(in_image[j * width + i]))*20;
			        }
            unsigned char output_val;
            if(temp_val > 255) output_val = 255;
            else output_val = temp_val;
            uchar4 output;
            output.x = output_val;
            output.y = output_val;
            output.z = output_val;
	          out_image[j * width + i] = output;
          }
      }
    double end = omp_get_wtime();
    time = (end - start) * 1000.0;
  }*/

void WaveletOpenmp::conv_f_gray_c_rgba(uchar4 *out_image, float *in_image, size_t width, size_t height, double &time)
  {
    size_t half_width = width / 2;
    size_t half_height = height / 2;
    double start = omp_get_wtime();
    #pragma omp parallel for default(none) shared(width, in_image, out_image, height, half_width, half_height)
    for(int j = 0; j < height; j++)
      {
        for(size_t i = 0; i < width; i++)
          {
			      float temp_val = abs(in_image[j * width + i]);
			      if((i >= half_width) || (j >= half_height))
			        {
                temp_val = log2(temp_val + 1)*20;
			        }
            unsigned char output_val;
            if(temp_val > 255) output_val = 255;
            else output_val = (unsigned char)(temp_val);
            uchar4 output;
            output.x = output_val;
            output.y = output_val;
            output.z = output_val;
            output.w = 0;
	          out_image[j * width + i] = output;
          }
      }
    double end = omp_get_wtime();
    time = (end - start) * 1000.0;
  }
