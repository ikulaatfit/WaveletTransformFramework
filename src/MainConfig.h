#ifndef MAIN_CONFIG_H
#define MAIN_CONFIG_H

#include <vector>
#include <ostream>
#include "WaveletOpenclParamSep.h"
#include "WaveletOpenclParamComb.h"
#include "WaveletOpenmpParamSep.h"
#include "Debug.h"
#include <ParseArg.h>
#include "main.h"

#include <WindowRenderer.h>
#include <FileReader.h>

#define DET_DEF_CAM_WIDTH     1280
#define DET_DEF_CAM_HEIGHT     720

enum e_output_type
{
  CONF_OUTPUT_TYPE_FULL = 0,
  CONF_OUTPUT_TYPE_CSV
};

/**
 * @brief Class for getting required data such as wald boost classifier file path, settings file path, input video file and optional data from program parameters.
 */
class MainConfig
  {
    public:
      std::string in_file; ///< Required input video file path
      std::string out_file; ///< Required output video file path
      int frame_count; ///< Frame count to process or DET_UNDEF
      bool print_devices; ///< Print devices
      bool in_show; ///< Input image show
      bool out_show; ///< Output image show
      bool benchmark_proc;
      bool memless_exec;
      int width;
      int height;
      bool fullscreen;
	  bool hw_decode;
      int screen;
      e_output_type output_type;
      engine_type engine;
      WaveletOpenclParamSep opencl_sep_par; ///< Program options
      WaveletOpenclParamComb opencl_comb_par; ///< Program options
      WaveletOpenmpParamSep openmp_sep_par; ///< Program options
      input_buffer_type buffer_t;
      e_reader_type reader_t;
	  e_renderer_type renderer_t;
      /**
       * Initialize object data.
       */
      MainConfig();
      /**
       * Delete object data.
       */
      ~MainConfig();
      /**
       * Load configuration from program parameters.
       * @param argc program parameters count
       * @param argv program parameters
       * @param error optional string to write error
       */
      bool loadFromArguments(int argc, char *argv[], std::ostream *error = NULL);
      /**
       * Print help.
       * @param exec_name program path
       */
      void printHelp(const char *exec_name);
    private:
      ParseArg parser; ///< Arguments parser
  };

#endif
