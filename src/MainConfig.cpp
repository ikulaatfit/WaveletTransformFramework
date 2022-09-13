#include "MainConfig.h"
enum e_main_opt
{
  MAIN_OPT_SHOW_INPUT,
  MAIN_OPT_SHOW_OUTPUT,
  MAIN_OPT_HELP,
  MAIN_OPT_INPUT,
  MAIN_OPT_DEVICE,
  MAIN_OPT_PRINT_DEVICES,
  MAIN_OPT_BENCHMARK_PROC,
  MAIN_OPT_DISABLE_GEN_FILTER_BODY,
  MAIN_OPT_FRAME_COUNT,
  MAIN_OPT_HOR_PAIRS,
  MAIN_OPT_VERT_PAIRS,
  MAIN_OPT_HOR_X,
  MAIN_OPT_HOR_Y,
  MAIN_OPT_VERT_X,
  MAIN_OPT_VERT_Y,
  MAIN_OPT_COMB_X,
  MAIN_OPT_COMB_Y,
  MAIN_OPT_HOR_KERNEL,
  MAIN_OPT_VERT_KERNEL,
  MAIN_OPT_COMB_KERNEL,
  MAIN_OPT_OUTPUT_TYPE,
  MAIN_OPT_COMB_CORN_PROC,
  MAIN_OPT_WARP_SIZE,
  MAIN_OPT_OPTIM_WARP,
  MAIN_OPT_OPTIM_THREAD,
  MAIN_OPT_HOR_PROC,
  MAIN_OPT_VERT_PROC,
  MAIN_OPT_ENGINE_TYPE,
  MAIN_OPT_HOR_FUNC,
  MAIN_OPT_VERT_FUNC,
  MAIN_OPT_INTERLACED,
  MAIN_OPT_MAX_DEPTH,
  MAIN_OPT_WAVELET_TYPE,
  MAIN_OPT_RESIZE_X,
  MAIN_OPT_RESIZE_Y,
  MAIN_OPT_SUBDEVICE_SIZE,
  MAIN_OPT_IMAGE_MEM,
  MAIN_OPT_DISABLE_DOUBLE_BUFFERING,
  MAIN_OPT_REPEAT_COUNT,
  MAIN_OPT_MEMLESS_EXEC,
  MAIN_OPT_BUFFER_TYPE,
  MAIN_OPT_READER_TYPE,
  MAIN_OPT_RENDERER_TYPE,
  MAIN_OPT_WIDTH,
  MAIN_OPT_HEIGHT,
  MAIN_OPT_SCREEN,
  MAIN_OPT_FULLSCREEN,
  MAIN_OPT_OUTPUT,
  MAIN_OPT_HW_DECODE_DISABLE
};


MainConfig::MainConfig()
{
  this->in_show = false;
  this->out_show = false;
  this->print_devices = false;
  this->frame_count = DET_UNDEF;
  this->output_type = CONF_OUTPUT_TYPE_FULL;
  this->engine = ENGINE_TYPE_OPENCL_COMB;
  this->memless_exec = false;
  this->benchmark_proc = false;
  this->reader_t = READER_TYPE_CAMERA;
  this->buffer_t = INPUT_BUFFER_TYPE_CPU;
  this->renderer_t = RENDERER_TYPE_SDL;
  this->width = DET_DEF_CAM_WIDTH;
  this->height = DET_DEF_CAM_HEIGHT;
  this->screen = DEF_WINDOW_NUM;
  this->fullscreen = false;
  this->out_file = "";
  this->hw_decode = true;
  std::vector<opt> opts{{MAIN_OPT_SHOW_INPUT, "show-input", 0, false, "Show input video file while converting"},
                       {MAIN_OPT_SHOW_OUTPUT, "show-output", 0, false, "Show output video file while converting"},
                       {MAIN_OPT_HELP, "help", 'h', false, "Print this help"},
                       {MAIN_OPT_INPUT, "input", 'i', true, "Set path to input image/video fileor camera number"},
                       {MAIN_OPT_DEVICE, "device", 'd', true, "Set device {c,cpu,g,gpu}"},
                       {MAIN_OPT_PRINT_DEVICES, "print-devices", 0, false, "Print devices info"},
                       {MAIN_OPT_BENCHMARK_PROC, "benchmark-proc", 0, false, "Benchmark process only execution (virtual data)"},
                       {MAIN_OPT_DISABLE_GEN_FILTER_BODY, "disable-gen-filter-body", 0, false, "Disable generation of filter body"},
                       {MAIN_OPT_FRAME_COUNT, "frame-count", 0, true, "Set frame process count"},
                       {MAIN_OPT_HOR_PAIRS, "hor-pairs", 0, true, "Pixel pairs to process by thread in horizontal transform"},
                       {MAIN_OPT_VERT_PAIRS, "vert-pairs", 0, true, "pixel pairs to process by thread in vertical transform"},
                       {MAIN_OPT_HOR_X, "hor-x", 0, true, "Width of group in horizontal transform"},
                       {MAIN_OPT_HOR_Y, "hor-y", 0, true, "Height of group in horizontal transform"},
                       {MAIN_OPT_VERT_X, "vert-x", 0, true, "Width of group in vertical transform"},
                       {MAIN_OPT_VERT_Y, "vert-y", 0, true, "Height of group in vertical transform"},
                       {MAIN_OPT_COMB_X, "comb-x", 0, true, "Width of group in combined transform"},
                       {MAIN_OPT_COMB_Y, "comb-y", 0, true, "Height of group in combined transform"},
                       {MAIN_OPT_HOR_KERNEL, "hor-kernel", 0, true, "Horizontal pass kernel name"},
                       {MAIN_OPT_VERT_KERNEL, "vert-kernel", 0, true, "Vertical pass kernel name"},
                       {MAIN_OPT_COMB_KERNEL, "comb-kernel", 0, true, "Combined pass kernel named"},
                       {MAIN_OPT_OUTPUT_TYPE, "output-type", 0, true, "Set output type {full,f,csv,c}"},
                       {MAIN_OPT_COMB_CORN_PROC, "comb-corn-proc", 0, false, "Process horizontal corners in comb out kernels"},
                       {MAIN_OPT_WARP_SIZE, "warp-size", 0, true, "Size of warp"},
                       {MAIN_OPT_OPTIM_WARP, "optim-warp", 0, true, "Optimize algorithm using atomic warp {none,n,local,l,shuffle,s}"},
                       {MAIN_OPT_OPTIM_THREAD, "optim-thread", 0, false, "Optimize algorithm using intra thread operations"},
                       {MAIN_OPT_HOR_PROC, "hor-proc", 0, true, "Set type of hor process {blaz_normal,bn,blaz_register,br,laan,l}"},
                       {MAIN_OPT_VERT_PROC, "vert-proc", 0, true, "Set type of vert process {blaz_normal,bn,blaz_register,br,laan,l}"},
                       {MAIN_OPT_ENGINE_TYPE, "engine-type", 0, true, "Set benchmark type {opencl,openmp,openclcomb}"},
                       {MAIN_OPT_HOR_FUNC, "hor-func", 0, true, "Horizontal pass function name in OpenMP"},
                       {MAIN_OPT_VERT_FUNC, "vert-func", 0, true, "Vertical pass function name in OpenMP"},
                       {MAIN_OPT_INTERLACED, "interlaced", 0, false, "Wavelet use output interlaced"},
                       {MAIN_OPT_MAX_DEPTH, "max-depth", 0, true, "Maximum depth of transformation" },
                       {MAIN_OPT_WAVELET_TYPE, "wavelet-type", 0, true, "Type of wavelet {CDF53,CDF97,DD137}"},
                       {MAIN_OPT_RESIZE_X, "resize-x", 0, true, "Resolution in x axis to resize"},
                       {MAIN_OPT_RESIZE_Y, "resize-y", 0, true, "Resolution in y axis to resize"},
                       {MAIN_OPT_SUBDEVICE_SIZE, "subdevice-size", 0, true, "Size of subdevice"},
                       {MAIN_OPT_IMAGE_MEM, "image-mem", 0, true, "Type of image memory {texture,global}"},
                       {MAIN_OPT_DISABLE_DOUBLE_BUFFERING, "disable-double-buffering", 0, false, "Disable double buffering"},
                       {MAIN_OPT_REPEAT_COUNT, "repeat-count", 0, true, "Wavelet execution repeat count <0,inf) (only for memless-exec)"},
                       {MAIN_OPT_MEMLESS_EXEC, "memless-exec", 0, false, "Memoryless execution"},
                       {MAIN_OPT_BUFFER_TYPE, "buffer-type", 0, true, "Buffer type (CPU|OPENGL)"},
                       {MAIN_OPT_READER_TYPE, "reader-type", 0, true, "Video reader type (VIDEO|CAMERA|OPENCV_VIDEO|OPENCV_CAMERA|FFMPEG_VIDEO|FFMPEG_CAMERA)"},
                       {MAIN_OPT_RENDERER_TYPE, "displayer-type", 0, true, "Video displayer type {OPENCV,SDL}"},
                       {MAIN_OPT_WIDTH, "width", 0, true, "Camera width"},
                       {MAIN_OPT_HEIGHT, "height", 0, true, "camera height"},
                       {MAIN_OPT_SCREEN, "screen", 0, true, "Screen number"},
                       {MAIN_OPT_FULLSCREEN, "fullscreen", 0, false, "Fullscreen"},
                       {MAIN_OPT_OUTPUT, "output", 'o', true, "Save output to file"},
					   {MAIN_OPT_HW_DECODE_DISABLE, "disable-hw-decode", 0, false, "Disable of hw acceleration for decoding."}};
  this->parser.addOpts(opts);
}

MainConfig::~MainConfig()
{}

void MainConfig::printHelp(const char *exec_name)
{
  std::string header = std::string() + "\nUsage: " + exec_name + " -i=input_video_file [parameters]\n";
  this->parser.printHelp(header);
}

bool MainConfig::loadFromArguments(int argc, char *argv[], std::ostream *error)
{
  std::string opt_value;
  int opt_num;
  int type_count = 0;
  int ival;
  for(int i = 1; i < argc; i++)
  {
    opt_num = parser.getArgOpt(argv[i], opt_value);
    switch(opt_num)
    {
    case MAIN_OPT_SHOW_INPUT:
      this->in_show = true;
      break;
    case MAIN_OPT_SHOW_OUTPUT:
      this->out_show = true;
      break;
    case MAIN_OPT_HELP:
      this->printHelp(argv[0]);
      exit(0);
      break;
    case MAIN_OPT_INPUT:
      if(opt_value.length() == 0)
      {
        if(error != NULL) *error << "Warning: Parameter \"input\" is empty.\n";
      }
      else
      {
        this->in_file = opt_value;
      }
      break;
    case MAIN_OPT_DEVICE:
      if((opt_value.compare("gpu") == 0) || (opt_value.compare("g") == 0))
      {
        this->opencl_comb_par.dev_id = 0;
        this->opencl_sep_par.dev_id = 0;
        this->opencl_comb_par.dev_type = CL_DEVICE_TYPE_GPU;
        this->opencl_sep_par.dev_type = CL_DEVICE_TYPE_GPU;
      }
      else if((opt_value.compare("cpu") == 0) || (opt_value.compare("c") == 0))
      {
        this->opencl_comb_par.dev_id = 0;
        this->opencl_sep_par.dev_id = 0;
        this->opencl_comb_par.dev_type = CL_DEVICE_TYPE_CPU;
        this->opencl_sep_par.dev_type = CL_DEVICE_TYPE_CPU;
      }
      else
      {
        ival = atoi(opt_value.c_str());
        if(ival < 0)
        {
          if(error != NULL) *error << "Warning: Parameter \"device\" is not correct. Right values are (gpu|g|prefer-gpu) for prefer gpu (cpu|c|prefer-cpu) for prefer cpu (force-gpu) for force gpu (force-cpu) for force cpu or device id.\n";
        }
        else
        {
          this->opencl_comb_par.dev_id = ival;
          this->opencl_sep_par.dev_id = ival;
          this->opencl_comb_par.dev_type = CL_DEVICE_TYPE_ALL;
          this->opencl_sep_par.dev_type = CL_DEVICE_TYPE_ALL;
        }
      }
      type_count++;
      break;
    case MAIN_OPT_PRINT_DEVICES:
      this->print_devices = true;
      break;
    case MAIN_OPT_BENCHMARK_PROC:
      this->opencl_comb_par.benchmark_proc = true;
      this->opencl_sep_par.benchmark_proc = true;
      this->benchmark_proc = true;
      break;
    case MAIN_OPT_DISABLE_GEN_FILTER_BODY:
      this->opencl_comb_par.gen_filter_body = false;
      this->opencl_sep_par.gen_filter_body = false;
      break;
    case MAIN_OPT_FRAME_COUNT:
      ival = atoi(opt_value.c_str());
      if(ival < 1)
      {
        if(error != NULL) *error << "Warning: Parameter \"frame-count\" is out of range. Right range is <1,kernel_count).\n";
      }
      else
      {
        this->frame_count = ival;
      }
      break;
    case MAIN_OPT_HOR_PAIRS:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 8))
      {
        if(error != NULL) *error << "Warning: Parameter \"hor-pairs\" is out of range. Right range is <1,8>.\n";
      }
      else
      {
        this->opencl_sep_par.pairs_per_thread.x = ival;
        this->opencl_comb_par.pairs_per_thread.x = ival;
      }
      break;
    case MAIN_OPT_VERT_PAIRS:
      ival = atoi(opt_value.c_str());
      if(ival < 1)
      {
        if(error != NULL) *error << "Warning: Parameter \"vert-pairs\" is out of range. Right range is <1,inf).\n";
      }
      else
      {
        this->opencl_sep_par.pairs_per_thread.y = ival;
        this->opencl_comb_par.pairs_per_thread.y = ival;
      }
      break;
    case MAIN_OPT_HOR_X:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 1024))
      {
        if(error != NULL) *error << "Warning: Parameter \"hor-x\" is out of range. Right range is <1,1024>.\n";
      }
      else
      {
        this->opencl_sep_par.hor_sizes.x = ival;
      }
      break;
    case MAIN_OPT_HOR_Y:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 1024))
      {
        if(error != NULL) *error << "Warning: Parameter \"hor-y\" is out of range. Right range is <1,1024>.\n";
      }
      else
      {
        this->opencl_sep_par.hor_sizes.y = ival;
      }
      break;
    case MAIN_OPT_VERT_X:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 1024))
      {
        if(error != NULL) *error << "Warning: Parameter \"vert-x\" is out of range. Right range is <1,1024>.\n";
      }
      else
      {
        this->opencl_sep_par.vert_sizes.x = ival;
      }
      break;
    case MAIN_OPT_VERT_Y:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 1024))
      {
        if(error != NULL) *error << "Warning: Parameter \"vert-y\" is out of range. Right range is <1,1024>.\n";
      }
      else
      {
        this->opencl_sep_par.vert_sizes.y = ival;
      }
      break;
    case MAIN_OPT_COMB_X:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 1024))
      {
        if(error != NULL) *error << "Warning: Parameter \"comb-x\" is out of range. Right range is <1,1024>.\n";
      }
      else
      {
        this->opencl_comb_par.comb_sizes.x = ival;
      }
      break;
    case MAIN_OPT_COMB_Y:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 1024))
      {
        if(error != NULL) *error << "Warning: Parameter \"comb-y\" is out of range. Right range is <1,1024>.\n";
      }
      else
      {
        this->opencl_comb_par.comb_sizes.y = ival;
      }
      break;
    case MAIN_OPT_HOR_KERNEL:
      this->opencl_sep_par.hor_kernel = opt_value;
      break;
    case MAIN_OPT_VERT_KERNEL:
      this->opencl_sep_par.vert_kernel = opt_value;
      break;
    case MAIN_OPT_COMB_KERNEL:
      this->opencl_comb_par.setKernel(opt_value);
      break;
    case MAIN_OPT_OUTPUT_TYPE:
      if((opt_value.compare("full") == 0) || (opt_value.compare("f") == 0))
      {
        this->output_type = CONF_OUTPUT_TYPE_FULL;
      }
      else if((opt_value.compare("csv") == 0) || (opt_value.compare("c") == 0))
      {
        this->output_type = CONF_OUTPUT_TYPE_CSV;
      }
      else
      {
        if(error != NULL) *error << "Warning: Unknown type of parameter \"output-type\". Supported types are (full|f) for full stdout mode (csv|c) for csv stdout mode.\n";
      }
      break;
    case MAIN_OPT_COMB_CORN_PROC:
      this->opencl_comb_par.comb_hor_corners_proc = 1;
      break;
    case MAIN_OPT_WARP_SIZE:
      ival = atoi(opt_value.c_str());
      if(ival < 1)
      {
        if(error != NULL) *error << "Warning: Parameter \"warp-size\" is out of range. Right range is <1,inf).\n";
      }
      else
      {
        this->opencl_sep_par.warp_size = ival;
        this->opencl_comb_par.warp_size = ival;
      }
      break;
    case MAIN_OPT_OPTIM_WARP:
      if((opt_value.compare("none") == 0) || (opt_value.compare("n") == 0))
      {
        this->opencl_sep_par.optim_warp = WAVELET_OPTIM_WARP_NONE;
        this->opencl_comb_par.optim_warp = WAVELET_OPTIM_WARP_NONE;
      }
      else if((opt_value.compare("local") == 0) || (opt_value.compare("l") == 0))
      {
        this->opencl_sep_par.optim_warp = WAVELET_OPTIM_WARP_LOCAL;
        this->opencl_comb_par.optim_warp = WAVELET_OPTIM_WARP_LOCAL;
      }
      else if((opt_value.compare("shuffle") == 0) || (opt_value.compare("s") == 0))
      {
        this->opencl_sep_par.optim_warp = WAVELET_OPTIM_WARP_SHUFFLE;
        this->opencl_comb_par.optim_warp = WAVELET_OPTIM_WARP_SHUFFLE;
      }
      else
      {
        if(error != NULL) *error << "Warning: Unknown type of parameter \"optim-warp\". Supported types are (none|n) for disable warp optimizalizations, (local|l) for optimalization using local memory or (shuffle|s) for optimalization using local memory.\n";
      }
      break;
    case MAIN_OPT_OPTIM_THREAD:
      this->opencl_comb_par.optim_thread = true;
      break;
    case MAIN_OPT_HOR_PROC:
      if((opt_value.compare("blaz_normal") == 0) || (opt_value.compare("bn") == 0))
      {
        this->opencl_sep_par.hor_proc = WAVELET_PROC_TYPE_BLAZ_NORMAL;
        this->opencl_comb_par.hor_proc = WAVELET_PROC_TYPE_BLAZ_NORMAL;
      }
      else if((opt_value.compare("blaz_register") == 0) || (opt_value.compare("br") == 0))
      {
        this->opencl_sep_par.hor_proc = WAVELET_PROC_TYPE_BLAZ_REGISTER;
        this->opencl_comb_par.hor_proc = WAVELET_PROC_TYPE_BLAZ_REGISTER;
      }
      else if((opt_value.compare("laan") == 0) || (opt_value.compare("l") == 0))
      {
        this->opencl_sep_par.hor_proc = WAVELET_PROC_TYPE_LAAN;
        this->opencl_comb_par.hor_proc = WAVELET_PROC_TYPE_LAAN;
      }
      else
      {
        if(error != NULL) *error << "Warning: Wrong value for parameter \"hor-proc\".\n";
      }
      break;
    case MAIN_OPT_VERT_PROC:
      if((opt_value.compare("blaz_normal") == 0) || (opt_value.compare("bn") == 0))
      {
        this->opencl_sep_par.vert_proc = WAVELET_PROC_TYPE_BLAZ_NORMAL;
        this->opencl_comb_par.vert_proc = WAVELET_PROC_TYPE_BLAZ_NORMAL;
      }
      else if((opt_value.compare("blaz_register") == 0) || (opt_value.compare("br") == 0))
      {
        this->opencl_sep_par.vert_proc = WAVELET_PROC_TYPE_BLAZ_REGISTER;
        this->opencl_comb_par.vert_proc = WAVELET_PROC_TYPE_BLAZ_REGISTER;
      }
      else if((opt_value.compare("laan") == 0) || (opt_value.compare("l") == 0))
      {
        this->opencl_sep_par.vert_proc = WAVELET_PROC_TYPE_LAAN;
        this->opencl_comb_par.vert_proc = WAVELET_PROC_TYPE_LAAN;
      }
      else
      {
        if(error != NULL) *error << "Warning: Wrong value for parameter \"vert-proc\".\n";
      }
      break;
    case MAIN_OPT_ENGINE_TYPE:
      if(opt_value.compare("opencl") == 0)
      {
        this->engine = ENGINE_TYPE_OPENCL_SEP;
      }
      else if(opt_value.compare("openmp") == 0)
      {
        this->engine = ENGINE_TYPE_OPENMP_SEP;
      }
      else if(opt_value.compare("openclcomb") == 0)
      {
        this->engine = ENGINE_TYPE_OPENCL_COMB;
      }
      else
      {
        if(error != NULL) *error << "Warning: Wrong value of parameter \"engine-type\".\n";
      }
      break;
    case MAIN_OPT_HOR_FUNC:
      if(opt_value.compare("transformSepSingle") == 0)
      {
        this->openmp_sep_par.hor_func = OPENMP_SEP_FUNC_SINGLE;
      }
      else if(opt_value.compare("transformSepOpenmp") == 0)
      {
        this->openmp_sep_par.hor_func = OPENMP_SEP_FUNC_OPENMP;
      }
      else if(opt_value.compare("transformSepOpenmpCopy") == 0)
      {
        this->openmp_sep_par.hor_func = OPENMP_SEP_FUNC_OPENMP_COPY;
      }
      else if(opt_value.compare("transformSepOpenmpSimd") == 0)
      {
        this->openmp_sep_par.hor_func = OPENMP_SEP_FUNC_OPENMP_SIMD;
      }
      else if(opt_value.compare("transformSepOpenmpCopySimd") == 0)
      {
        this->openmp_sep_par.hor_func = OPENMP_SEP_FUNC_OPENMP_COPY_SIMD;
      }
      else
      {
        if(error != NULL) *error << "Warning: Wrong value of parameter \"hor-func\".\n";
      }
      break;
    case MAIN_OPT_VERT_FUNC:
      if(opt_value.compare("transformSepSingle") == 0)
      {
        this->openmp_sep_par.vert_func = OPENMP_SEP_FUNC_SINGLE;
      }
      else if(opt_value.compare("transformSepOpenmp") == 0)
      {
        this->openmp_sep_par.vert_func = OPENMP_SEP_FUNC_OPENMP;
      }
      else if(opt_value.compare("transformSepOpenmpCopy") == 0)
      {
        this->openmp_sep_par.vert_func = OPENMP_SEP_FUNC_OPENMP_COPY;
      }
      else if(opt_value.compare("transformSepOpenmpSimd") == 0)
      {
        this->openmp_sep_par.vert_func = OPENMP_SEP_FUNC_OPENMP_SIMD;
      }
      else if(opt_value.compare("transformSepOpenmpCopySimd") == 0)
      {
        this->openmp_sep_par.vert_func = OPENMP_SEP_FUNC_OPENMP_COPY_SIMD;
      }
      else
      {
        if(error != NULL) *error << "Warning: Wrong value of parameter \"vert-func\".\n";
      }
      break;
    case MAIN_OPT_INTERLACED:
      this->opencl_comb_par.interlaced = true;
      this->opencl_sep_par.interlaced = true;
      this->openmp_sep_par.interlaced = true;
      break;
    case MAIN_OPT_MAX_DEPTH:
      ival = atoi(opt_value.c_str());
      if(ival < 1)
      {
        if(error != NULL) *error << "Warning: Parameter \"depth\" is out of range. Right range is <1,inf>.\n";
      }
      else
      {
        this->opencl_comb_par.max_depth = ival;
        this->opencl_sep_par.max_depth = ival;
        this->openmp_sep_par.max_depth = ival;
      }
      break;
    case MAIN_OPT_WAVELET_TYPE:
      if(opt_value.compare("CDF53") == 0)
      {
        this->opencl_comb_par.setWaveletType(WAVELET_TYPE_CDF53);
        this->opencl_sep_par.setWaveletType(WAVELET_TYPE_CDF53);
        this->openmp_sep_par.setWaveletType(WAVELET_TYPE_CDF53);
      }
      else if(opt_value.compare("CDF97") == 0)
      {
        this->opencl_comb_par.setWaveletType(WAVELET_TYPE_CDF97);
        this->opencl_sep_par.setWaveletType(WAVELET_TYPE_CDF97);
        this->openmp_sep_par.setWaveletType(WAVELET_TYPE_CDF97);
      }
      else if(opt_value.compare("DD137") == 0)
      {
        this->opencl_comb_par.setWaveletType(WAVELET_TYPE_DD137);
        this->opencl_sep_par.setWaveletType(WAVELET_TYPE_DD137);
        this->openmp_sep_par.setWaveletType(WAVELET_TYPE_DD137);
      }
	  else if (opt_value.compare("N2521") == 0)
	  {
		  this->opencl_comb_par.setWaveletType(WAVELET_TYPE_N2521);
		  this->opencl_sep_par.setWaveletType(WAVELET_TYPE_N2521);
		  this->openmp_sep_par.setWaveletType(WAVELET_TYPE_N2521);
	  }
	  else if (opt_value.compare("N1715") == 0)
	  {
		  this->opencl_comb_par.setWaveletType(WAVELET_TYPE_N1715);
		  this->opencl_sep_par.setWaveletType(WAVELET_TYPE_N1715);
		  this->openmp_sep_par.setWaveletType(WAVELET_TYPE_N1715);
	  }
	  else if (opt_value.compare("N2523") == 0)
	  {
		  this->opencl_comb_par.setWaveletType(WAVELET_TYPE_N2523);
		  this->opencl_sep_par.setWaveletType(WAVELET_TYPE_N2523);
		  this->openmp_sep_par.setWaveletType(WAVELET_TYPE_N2523);
	  }
      else
      {
        if(error != NULL) *error << "Warning: Unknown type of parameter \"wavelet-type\". Supported types are (CDF53|CDF97|DD137).\n";
      }
      break;
    case MAIN_OPT_RESIZE_X:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 50000))
      {
        if(error != NULL) *error << "Warning: Parameter \"resize-x\" is out of range. Right range is <1,50000>.\n";
      }
      else
      {
        this->openmp_sep_par.scaled_image_size.x = ival;
        this->opencl_sep_par.scaled_image_size.x = ival;
        this->opencl_comb_par.scaled_image_size.x = ival;
      }
      break;
    case MAIN_OPT_RESIZE_Y:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 50000))
      {
        if(error != NULL) *error << "Warning: Parameter \"resize-y\" is out of range. Right range is <1,50000>.\n";
      }
      else
      {
        this->openmp_sep_par.scaled_image_size.y = ival;
        this->opencl_sep_par.scaled_image_size.y = ival;
        this->opencl_comb_par.scaled_image_size.y = ival;
      }
      break;
    case MAIN_OPT_SUBDEVICE_SIZE:
      ival = atoi(opt_value.c_str());
      if((ival < 0) || (ival > 5000))
      {
        if(error != NULL) *error << "Warning: Parameter \"subdevice-size\" is out of range. Right range is <1,inf) and 0 for use of whole device.\n";
      }
      else
      {
        this->opencl_comb_par.subdevice_size = ival;
        this->opencl_sep_par.subdevice_size = ival;
      }
      break;
    case MAIN_OPT_IMAGE_MEM:
      if(opt_value.compare("texture") == 0)
      {
        this->opencl_comb_par.image_mem_type = OPENCL_MEM_TYPE_TEXTURE;
        this->opencl_sep_par.image_mem_type = OPENCL_MEM_TYPE_TEXTURE;
      }
      else if(opt_value.compare("global") == 0)
      {
        this->opencl_comb_par.image_mem_type = OPENCL_MEM_TYPE_GLOBAL;
        this->opencl_sep_par.image_mem_type = OPENCL_MEM_TYPE_GLOBAL;
      }
      else
      {
        if(error != NULL) *error << "Warning: Unknown type of parameter \"image-mem\". Supported types are (global|texture).\n";
      }
      break;
    case MAIN_OPT_DISABLE_DOUBLE_BUFFERING:
      this->opencl_comb_par.double_buffering = false;
      this->opencl_sep_par.double_buffering = false;
      break;
    case MAIN_OPT_REPEAT_COUNT:
      ival = atoi(opt_value.c_str());
      if((ival < 1) || (ival > 5000))
      {
        if(error != NULL) *error << "Warning: Parameter \"repeat-count\" is out of range. Right range is <0,inf).\n";
      }
      else
      {
        this->opencl_comb_par.repeat_count = ival;
        this->opencl_sep_par.repeat_count = ival;
      }
      break;
    case MAIN_OPT_MEMLESS_EXEC:
      this->opencl_comb_par.memless_exec = true;
      this->opencl_sep_par.memless_exec = true;
      this->memless_exec = true;
      break;
    case MAIN_OPT_BUFFER_TYPE:
      if(opt_value.compare("CPU") == 0)
      {
        this->buffer_t = INPUT_BUFFER_TYPE_CPU;
      }
      else if(opt_value.compare("OPENGL") == 0)
      {
        this->buffer_t = INPUT_BUFFER_TYPE_OPENGL;
      }
      else
      {
        if(error != NULL) *error << "Warning: Unknown type of parameter \"buffer-type\". Supported types are (CPU|OPENGL).\n";
      }
      break;
    case MAIN_OPT_READER_TYPE:
		if (opt_value.compare("VIDEO") == 0)
		{
			this->reader_t = READER_TYPE_VIDEO;
		}
		else if (opt_value.compare("CAMERA") == 0)
		{
			this->reader_t = READER_TYPE_CAMERA;
		}
#ifdef OPENCV_ENABLED
      else if(opt_value.compare("OPENCV_VIDEO") == 0)
      {
        this->reader_t = READER_TYPE_OPENCV_VIDEO;
      }
      else if(opt_value.compare("OPENCV_CAMERA") == 0)
      {
        this->reader_t = READER_TYPE_OPENCV_CAMERA;
      }
#endif
#ifdef FFMPEG_ENABLED
	  else if (opt_value.compare("FFMPEG_VIDEO") == 0)
	  {
		  this->reader_t = READER_TYPE_FFMPEG_VIDEO;
	  }
	  else if (opt_value.compare("FFMPEG_CAMERA") == 0)
	  {
		  this->reader_t = READER_TYPE_FFMPEG_CAMERA;
	  }
#endif
      else
      {
        if(error != NULL) *error << "Warning: Parameter \"reader-type\" is not correct. Right values are (VIDEO|CAMERA|OPENCV_VIDEO|OPENCV_CAMERA|FFMPEG_VIDEO|FFMPEG_CAMERA)\n";
      }
      break;
    case MAIN_OPT_RENDERER_TYPE:
		if (opt_value.compare("OPENCV") == 0)
		{
#ifdef OPENCV_ENABLED
			this->renderer_t = RENDERER_TYPE_OPENCV;
#endif
		}
		else if (opt_value.compare("SDL") == 0)
		{
#ifdef SDL_ENABLED
			this->renderer_t = RENDERER_TYPE_SDL;
#endif
		}
		else
      {
        if(error != NULL) *error << "Warning: Parameter \"display-type\" is not correct. Right values are (OPENCV|SDL)\n";
      }
      break;
    case MAIN_OPT_WIDTH:
      ival = atoi(opt_value.c_str());
      if(ival <= 0)
      {
        if(error != NULL) *error << "Warning: Parameter \"width\" is out of range. Right range is <1,inf).\n";
      }
      else
      {
        this->width = ival;
      }
      break;
    case MAIN_OPT_HEIGHT:
      ival = atoi(opt_value.c_str());
      if(ival <= 0)
      {
        if(error != NULL) *error << "Warning: Parameter \"height\" is out of range. Right range is <1,inf).\n";
      }
      else
      {
        this->height = ival;
      }
      break;
    case MAIN_OPT_SCREEN:
      ival = atoi(opt_value.c_str());
      if(ival < 0)
      {
        if(error != NULL) *error << "Warning: Parameter \"screen\" is out of range. Right range is <0,inf).\n";
      }
      else
      {
        this->screen = ival;
      }
      break;
    case MAIN_OPT_FULLSCREEN:
      this->fullscreen = true;
      break;
	case MAIN_OPT_HW_DECODE_DISABLE:
		this->hw_decode = false;
		break;
    case MAIN_OPT_OUTPUT:
      this->out_file = opt_value;
      break;
    case PARSE_ARG_NOT_FOUND:
      if(error != NULL) *error << "Warning: Parameter \"" << argv[i] << "\" not found.\n";
    }
  }
  if(this->memless_exec)
  {
    this->opencl_comb_par.benchmark_proc = true;
    this->opencl_sep_par.benchmark_proc = true;
    this->benchmark_proc = true;
  }
  if(this->benchmark_proc && (!this->opencl_comb_par.scaled_image_size.isValid()))
  {
    if(error != NULL) *error << "Warning: Calculation without input cannot be used without set parameters resize-x and resize-y.\n";
    return false;
  }
  if(type_count > 1)
  {
    if(error != NULL) *error << "Warning: Cannot have more than 1 device type specifiers. Uses last one.\n";
  }
  if((this->in_file.length() == 0) && (!this->print_devices) && (!this->benchmark_proc))
  {
    if(error != NULL) *error << "Warning: No input file or camera specified. Default camera used.\n";
    this->reader_t = READER_TYPE_CAMERA;
    this->in_file = "0";
  }
  return true;
}
