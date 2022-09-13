#include <FileReader.h>
#include <FileWriter.h>
#include <WindowRenderer.h>

#include "main.h"
#include "WaveletOpenmpSep.h"
#include "WaveletOpenclSep.h"
#include "WaveletOpenclComb.h"
#include "MainConfig.h"
#include <sstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Debug.h"
#include <omp.h>



#define MINIMUM_FRAME_TIME 0.0f

size_t ceil_div(size_t num, size_t block)
{
  return (num + block - 1) / block;
}

size_t ceil_align(size_t num, size_t block)
  {
    return ceil_div(num, block) * block;
  }

double get_time(cl_event proc_event)
  {
    cl_ulong start, end;
    clGetEventProfilingInfo(proc_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), (void *)&start, NULL);
    clGetEventProfilingInfo(proc_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), (void *)&end, NULL);
    return (end - start)/1000000.0;
  }

void gray_to_rgba(cl_uchar4 *output, cl_uchar *input, unsigned int input_size)
{
	for (int i = 0; i < input_size; i++)
	{
		output[i] = { input[i],input[i],input[i],0 };
	}
}

void print_device_info(cl_uint device_num, cl_device_id id, FILE *file)
{
    size_t par_size;
    cl_platform_id platform;
    cl_uint subdevices_count;
    cl_uint compute_units_count;

    // get device name
    clGetDeviceInfo(id, CL_DEVICE_NAME, 0, NULL, &par_size);
    char *device_name = new char[par_size + 1];
    clGetDeviceInfo(id, CL_DEVICE_NAME, par_size, (void *)device_name, NULL);
    device_name[par_size] = '\0';

    clGetDeviceInfo(id, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(cl_uint), (void*)&subdevices_count, NULL);
    clGetDeviceInfo(id, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), (void *)(&platform), NULL);
    clGetDeviceInfo(id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), (void *)(&compute_units_count), NULL);

    // get platform name
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &par_size);
    char *platform_name = new char[par_size + 1];
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, par_size, (void *)platform_name, NULL);
    platform_name[par_size] = '\0';

    fprintf(file, "\nDevice info:\n" \
                      " Device id:       %d\n" \
                      " Subdevices count %d\n" \
                      " Multiprocessors: %d\n" \
                      " Platform name:   %s\n" \
                      " Device name:     %s\n", device_num, subdevices_count, compute_units_count, platform_name, device_name);
    delete[] platform_name;
    delete[] device_name;
}

void print_error(std::stringstream error, char *label, char *exit_message = NULL)
{
  if(error.str().compare("") != 0)
  {
    fprintf(stderr, "\n%s:\n", label);
    fprintf(stderr, error.str().c_str());
    if(exit_message != NULL) fprintf(stderr, "\nExiting program: %s\n", exit_message);
    error.clear();
  }
}

void print_detection_info(e_output_type output_type, det_output timer, int frame_count)
{
  fprintf(stderr, "\nDetection info:\n" \
                    "  Processed %d frames.\n" \
                    "\nDetection time info:\n" \
                    "  Average detection time   %9.4f ms  consists of\n" \
                    "    Average preprocess time  %9.4f ms\n" \
                    "    Average process time     %9.4f ms\n" \
                    "    Average postprocess time %9.4f ms\n", frame_count, timer.getTotalTime()*1000.0, timer.getTotalPreprocessTime()*1000.0, timer.getTotalProcessTime()*1000.0, timer.getTotalPostprocessTime()*1000.0);
  fprintf(stderr, "\nDetection preprocess info:\n");
  for(unsigned int i = 0; i < timer.pre.size(); i++)
  {
    fprintf(stderr, "  Average kernel rank %3d time %9.4f ms\n", i, timer.pre[i] * 1000.0);
  }
  fprintf(stderr, "\nDetection process info:\n");
  for(unsigned int i = 0; i < timer.proc.size(); i++)
  {
    fprintf(stderr, "  Average kernel rank %3d time %9.4f ms\n", i, timer.proc[i] * 1000.0);
  }
  fprintf(stderr, "\nDetection postprocess info:\n");
  for(unsigned int i = 0; i < timer.post.size(); i++)
  {
    fprintf(stderr, "  Average kernel rank %3d time %9.4f ms\n", i, timer.post[i] * 1000.0);
  }
  switch(output_type)
  {
    case CONF_OUTPUT_TYPE_CSV:
      fprintf(stderr, "\nBenchmark Data:\n");
      fprintf(stdout, "%d\n%.4f\n%.4f;%.4f;%.4f\n", frame_count, timer.getTotalTime()*1000.0, timer.getTotalPreprocessTime()*1000.0, timer.getTotalProcessTime()*1000.0, timer.getTotalPostprocessTime()*1000.0);
      for(unsigned int i = 0; i < timer.pre.size(); i++)
        {
          if(i != 0) fprintf(stdout, ";");
          fprintf(stdout, "%.4f", timer.pre[i] * 1000.0);
        }
      fprintf(stdout, "\n");
      for(unsigned int i = 0; i < timer.proc.size(); i++)
        {
          if(i != 0) fprintf(stdout, ";");
          fprintf(stdout, "%.4f", timer.proc[i] * 1000.0);
        }
      fprintf(stdout, "\n");
      for(unsigned int i = 0; i < timer.post.size(); i++)
        {
          if(i != 0) fprintf(stdout, ";");
          fprintf(stdout, "%.4f", timer.post[i] * 1000.0);
        }
      fprintf(stdout, "\n");
    break;
    default:
    break;
  }
}

int main(int argc, char *argv[])
{
	std::ostream *error = &std::cerr;
	unsigned int width, height;
  FileReaderInterface *reader = NULL;
  FileWriterInterface *writer = NULL;
  WindowRendererInterface *input_displayer = NULL;
  WindowRendererInterface *output_displayer = NULL;
  Wavelet *wavelet = NULL;

  MainConfig m_conf;
  if(!m_conf.loadFromArguments(argc, argv, error))
  {
    exit(1);
  }

  if(m_conf.print_devices)
  {
    WaveletOpencl::printDevicesInfo(error, stdout);
    exit(0);
  }

  width = m_conf.width;
  height = m_conf.height;

  double fps;
  if(m_conf.benchmark_proc || m_conf.memless_exec)
  {
    if((!m_conf.in_show) && (!m_conf.out_show)) reader = new NoneReader(m_conf.in_file, width, height, fps, true, false, error);
    else reader = new EmptyReader(m_conf.in_file, width, height, fps, true, false, error);
    if(!reader->isOpened())
    {
      delete reader;
      exit(3);
    }
  }
  else
  {
    reader = createFileReader(m_conf.reader_t, m_conf.in_file, width, height, fps, false, false, m_conf.hw_decode, error);
    if(reader == NULL) exit(3);
  }
  
  bool is_out_save = m_conf.out_file.compare("") != 0;
  if(is_out_save)
  {
    writer = createFileWriter(m_conf.out_file, width, height, (fps <= 1) ? 15 : fps, error);
    if(writer == NULL) exit(3);
  }
  
  rect image_size(width, height);
    
  switch(m_conf.engine)
  {
    case ENGINE_TYPE_OPENCL_SEP:
      wavelet = new WaveletOpenclSep(image_size, &(m_conf.opencl_sep_par));
      break;
    case ENGINE_TYPE_OPENCL_COMB:
      wavelet = new WaveletOpenclComb(image_size, &(m_conf.opencl_comb_par));
      break;
    case ENGINE_TYPE_OPENMP_SEP:
      wavelet = new WaveletOpenmpSep(image_size, &(m_conf.openmp_sep_par));
      break;
  }

  std::vector<cl_uchar4> bgra_frame_data;
  bgra_frame_data.resize(image_size.getSize());
  /*(std::vector<cl_uchar> gray_frame;
  gray_frame.resize(image_size.getSize());*/

  if(wavelet == NULL)
  {
    exit(5);
  }
  if(!wavelet->isValid())
  {
    wavelet->printError();
		exit(6);
  }
  if(m_conf.in_show)
  {
    std::string window_label("Input");
    input_displayer = createOutputRenderer(m_conf.renderer_t, window_label, width, height, INPUT_BUFFER_TYPE_CPU, m_conf.screen, m_conf.fullscreen, s_color(0,255,0,0), error);
    if(input_displayer == NULL) exit(1);
	//input_frame_data.resize(width*height);
  }
  if(m_conf.out_show)
  {
    std::string window_label("Output");
    output_displayer = createOutputRenderer(m_conf.renderer_t, window_label, width, height, INPUT_BUFFER_TYPE_CPU, m_conf.screen, m_conf.fullscreen, s_color(0, 255, 0, 0), error);
    if(output_displayer == NULL) exit(1);
	//output_frame_data.resize(width*height);
  }
  det_output timer;
  det_output_stat timers;
  bool end = false;
  int i;
  std::stringstream frame_info;
  const char * key_info = "Quit:q";
  unsigned char key;
  bool process_stopped = false;
  bool first;
  double start_time = omp_get_wtime();
  double in_last_update = start_time - MINIMUM_FRAME_TIME;
  double out_last_update = start_time - MINIMUM_FRAME_TIME;
  double act_proc_time = 0;
  //for(i = 0; reader->getFrame(gray_frame.data(), error); i++)
  for (i = 0; reader->getFrame(bgra_frame_data.data(), error); i++)
	{
	  //gray_to_rgba(bgra_frame_data.data(), gray_frame.data(), image_size.getSize());
    timer.reset();
    if(i == m_conf.frame_count)
    {
      break;
    }
    if((m_conf.in_show) && (omp_get_wtime() - in_last_update >= MINIMUM_FRAME_TIME))
		{
      std::string empty_string;

      input_displayer->showFrame(bgra_frame_data.data(), empty_string);
      in_last_update = omp_get_wtime();
      if((key = input_displayer->getKey()) >= 0)
      {
        switch(key)
        {
          case 'q':
            i++;
            end = true;
            break;
        }
      }
		}
    if(!wavelet->getFrame((uchar4 *)bgra_frame_data.data(), &timer))
    {
      wavelet->printError();
      exit(9);
    }
    if(is_out_save && (!writer->write(bgra_frame_data.data(), error)))
    {
      exit(10);
    }
    timers.output.push_back(timer);
    if((m_conf.out_show) && (omp_get_wtime() - out_last_update >= MINIMUM_FRAME_TIME))
		{
      frame_info.str(std::string());
      frame_info << key_info << std::endl;
      switch(m_conf.engine)
      {
        case ENGINE_TYPE_OPENCL_SEP:
        case ENGINE_TYPE_OPENCL_COMB:
          if(m_conf.memless_exec || m_conf.benchmark_proc) frame_info << " Process time:   " << timer.getTotalProcessTime()*1000.0 << "ms" << std::endl;
          else frame_info << std::fixed << std::setprecision(4) << " Copy time:        " << (timer.pre[0] + timer.post[1])*1000.0 << "ms" << std::endl
                                                                << " Preprocess time:  " << timer.pre[1] * 1000.0 << "ms" << std::endl
                                                                << " Process time:     " << timer.getTotalProcessTime()*1000.0 << "ms" << std::endl
                                                                << " Postprocess time: " << timer.post[0] * 1000.0 << "ms" << std::endl
                                                                << " Total time:       " << timer.getTotalTime()*1000.0 << "ms";
        break;
        case ENGINE_TYPE_OPENMP_SEP:
        case ENGINE_TYPE_OPENMP_COMB:
          frame_info << std::fixed << std::setprecision(4) << " Preprocess time:  " << timer.getTotalPreprocessTime()*1000.0 << "ms" << std::endl
                                                            << " Process time:     " << timer.getTotalProcessTime()*1000.0 << "ms" << std::endl
                                                            << " Postprocess time: " << timer.getTotalPostprocessTime()*1000.0 << "ms" << std::endl
                                                            << " Total time:       " << timer.getTotalTime()*1000.0 << "ms";
        break;
      }
      std::string frame_info_str = frame_info.str();
      output_displayer->showFrame(bgra_frame_data.data(), frame_info_str, 10, 10);
      out_last_update = omp_get_wtime();
      process_stopped = true;
      first = true;
      while(process_stopped)
      {
        if(first)
        {
          process_stopped = false;
          first = false;
        }
        if((key = output_displayer->getKey()) >= 0)
				{
          switch(key)
            {
              case 'q':
                i++;
                end = true;
              break;
              case ' ':
                process_stopped = !process_stopped;
              break;
            }
				}
        if(end)
        {
          break;
        }
      }
		}
    if(end)
    {
      break;
    }
	}
  double end_time = omp_get_wtime();

  det_output avg_timer = timers.getMedianData();
  print_detection_info(m_conf.output_type, avg_timer, i);
  fprintf(stderr, "\nAvg frame time with overhead: %.4f ms\n", ((end_time - start_time)*1000.0) / ((double)(i)));
    
  if(input_displayer != NULL) delete input_displayer;
  if(output_displayer != NULL) delete output_displayer;
  if(wavelet != NULL) delete wavelet;
  if(writer != NULL) delete writer;
  if(reader != NULL) delete reader;

  return 0;
}
