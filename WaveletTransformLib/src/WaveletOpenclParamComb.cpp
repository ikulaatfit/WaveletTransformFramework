#include "WaveletOpenclParamComb.h"
#include <map>
#include <cfloat>
#include "WaveletKernelGeneratorCpu.h"

WaveletOpenclParamComb::WaveletOpenclParamComb() : WaveletOpenclParam(ENGINE_TYPE_OPENCL_COMB)
{
	this->kernel_generator = NULL;
	this->clear();
}
/**
* Set initial undefined value.
*/
void WaveletOpenclParamComb::clear()
{
	std::string def_kernel_name("wavelet_block_in_explosive");
	this->comb_sizes = proc_dim(32, 8);
	this->setKernel(def_kernel_name);
	this->comb_hor_corners_proc = 0;
	this->optim_thread = false;
	this->gen_data = std::string("");
	this->instrinsics = INTRINSICS_NONE;
	if (this->kernel_generator != NULL) delete this->kernel_generator;
	this->kernel_generator = NULL;
}


/**
  * Get program build parameters.
	* @param block_size threads in block count
	* @param stat_creating statistics creating type
	* @return parameters
  */
std::string WaveletOpenclParamComb::createBuildParam()
{
	std::ostringstream str("");
	str << this->WaveletOpenclParam::createBuildParam();
	str << " -D COMB_BLOCK_SIZE=" << this->comb_sizes.count()
		<< " -D COMB_BLOCK_SIZE_X=" << this->comb_sizes.x
		<< " -D COMB_HOR_CORNERS_PROC=" << this->comb_hor_corners_proc
		<< " -D " << this->comb_kernel
		<< this->getFirDefs();
	if (this->kernel_generator != NULL)
	{
		str << " -D BORDER_LINES_L=" << -this->kernel_generator->local_borders.min_b[0]
			<< " -D BORDER_LINES_T=" << -this->kernel_generator->local_borders.min_b[1]
			<< " -D BORDER_LINES_R=" << this->kernel_generator->local_borders.max_b[0]
			<< " -D BORDER_LINES_B=" << this->kernel_generator->local_borders.max_b[1]
			<< " -D BLOCK_IN_LOCAL_BLOCKS=" << (this->kernel_generator->local_borders.max_pos_id + 1);
	}

#if DEBUG_LEVEL == DEBUG_ALL
	this->printDebug();
#endif
	//fprintf(stderr, str.str().c_str());
	return str.str();
}
/**
  * Print debug information
  */
void WaveletOpenclParamComb::printDebug()
{
	unsigned int border_size = 2 * (this->wavelet_info->width * 2 - 1) * this->wavelet_info->steps;
	proc_dim in_pairs_per_group = comb_sizes * pairs_per_thread;
	proc_dim out_pairs_per_group = comb_sizes * pairs_per_thread - proc_dim(border_size, border_size, 0);
	float overhead = ((in_pairs_per_group.count() / (float)out_pairs_per_group.count())*100.0) - 100.0;
	fprintf(stderr, " comb kernel name:        %s\n"
		" thread optimalization:   %s\n"
		" comb group size:         %dx%d\n"
		" comb pairs per thread:   %dx%d\n"
		" comb pairs per group:    %dx%d\n"
		" comb out pairs per group:%dx%d\n"
		" overhead:                %.2f%%\n"
		" comb corners process:    %s\n", this->comb_kernel.c_str(), this->optim_thread ? "true" : "false", this->comb_sizes.x, this->comb_sizes.y, this->pairs_per_thread.x, this->pairs_per_thread.y, in_pairs_per_group.x, in_pairs_per_group.y, out_pairs_per_group.x, out_pairs_per_group.y, overhead, (this->comb_hor_corners_proc == 1) ? "yes" : "no");
	if(this->kernel_generator != NULL)
	{
		fprintf(stderr, " local memory borders:    x:<%d,%d> y:<%d,%d>\n"
			" local memory size:       %d\n"
			" op count                 %d\n"
			" load count               %d\n"
			" store count              %d\n"
			" shuffle count            %d\n"
			" barrier count            %d\n", this->kernel_generator->local_borders.min_b[0], this->kernel_generator->local_borders.max_b[0], this->kernel_generator->local_borders.min_b[1], this->kernel_generator->local_borders.max_b[1], (this->kernel_generator->local_borders.max_pos_id + 1), this->kernel_generator->op_count, this->kernel_generator->load_count, this->kernel_generator->store_count, this->kernel_generator->shuffle_count, this->kernel_generator->barrier_count);
	}

	std::string file_name(this->comb_kernel);
	if (this->optim_thread) file_name += std::string("_improved");
	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_NONE:
		file_name += std::string("_none");
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		file_name += std::string("_local");
		break;
	case WAVELET_OPTIM_WARP_SHUFFLE:
		file_name += std::string("_shuffle");
		break;
	}

	file_name += "_" + this->wavelet_info->define.substr(this->wavelet_info->define.find_last_of('_') + 1);
	file_name += "_" + std::to_string(this->pairs_per_thread.x) + "x" + std::to_string(this->pairs_per_thread.y);

	FILE *test = fopen((file_name + std::string(".txt")).c_str(), "w+");
	fprintf(test, (this->gen_data).c_str());
	fclose(test);
}

std::string WaveletOpenclParamComb::getFirDef(std::vector<float> &fir, std::string &filter_name)
{
	std::stringstream out_data;
	out_data.setf(std::ios_base::showpoint);
	out_data << " -D " << filter_name << "={";
	for (int i = 0; i < fir.size(); i++)
	{
		if (i != 0) out_data << ",";
		out_data << fir[i] << "f";
	}
	out_data << "}";
	return out_data.str();
}



bool WaveletOpenclParamComb::setKernel(std::string &kernel_name)
{
	if (kernel_name.empty()) return false;

	if (kernel_name.find("sweldens") != std::string::npos) this->kernel_type = WAVELET_KERNEL_SWELDENS;
	else if (kernel_name.find("iwahashi") != std::string::npos) this->kernel_type = WAVELET_KERNEL_IWAHASHI;
	else if (kernel_name.find("explosive") != std::string::npos) this->kernel_type = WAVELET_KERNEL_EXPLOSIVE;
	else if (kernel_name.find("monolithic") != std::string::npos) this->kernel_type = WAVELET_KERNEL_MONOLITHIC;
	else if (kernel_name.find("polyphase_sep_none") != std::string::npos) this->kernel_type = WAVELET_KERNEL_POLYPHASE_SEP_NONE;
	else if (kernel_name.find("polyphase_sep_hor") != std::string::npos) this->kernel_type = WAVELET_KERNEL_POLYPHASE_SEP_HOR;
	else if (kernel_name.find("polyphase_sep_vert") != std::string::npos) this->kernel_type = WAVELET_KERNEL_POLYPHASE_SEP_VERT;
	else if (kernel_name.find("polyphase_sep_all") != std::string::npos) this->kernel_type = WAVELET_KERNEL_POLYPHASE_SEP_ALL;
	else if (kernel_name.find("polyphase") != std::string::npos) this->kernel_type = WAVELET_KERNEL_POLYPHASE;
	else if (kernel_name.find("convolution_sep_none") != std::string::npos) this->kernel_type = WAVELET_KERNEL_CONVOLUTION_SEP_NONE;
	else if (kernel_name.find("convolution_sep_hor") != std::string::npos) this->kernel_type = WAVELET_KERNEL_CONVOLUTION_SEP_HOR;
	else if (kernel_name.find("convolution_sep_vert") != std::string::npos) this->kernel_type = WAVELET_KERNEL_CONVOLUTION_SEP_VERT;
	else if (kernel_name.find("convolution_sep_all") != std::string::npos) this->kernel_type = WAVELET_KERNEL_CONVOLUTION_SEP_ALL;
	else if (kernel_name.find("convolution") != std::string::npos) this->kernel_type = WAVELET_KERNEL_CONVOLUTION;
	else this->kernel_type = WAVELET_KERNEL_OTHER;

	this->comb_kernel = kernel_name;
	return true;
}

bool WaveletOpenclParamComb::isHorOpAtomic()
{
	//return (this->warp_size % (this->comb_sizes.x * this->pairs_per_thread.x) == 0);
	return (this->warp_size % this->comb_sizes.x) == 0;
}

std::string WaveletOpenclParamComb::getFirDefs()
{
	if ((!this->isHorOpAtomic()) && (this->optim_warp != WAVELET_OPTIM_WARP_NONE))
	{
		this->optim_warp = WAVELET_OPTIM_WARP_NONE;
		fprintf(stderr, "Warp size is smaller than number of threads in row. Setting warp optimization to disabled.");
	}
	std::ostringstream str("");
	str << " -D OPTIM_THREAD=" << ((this->optim_thread) ? "1" : "0");
	if ((this->gen_filter_body) && (this->kernel_type != WAVELET_KERNEL_OTHER))
	{
		//this->kernel_generator = new WaveletKernelGeneratorGpu({ (int)comb_sizes.x, (int)comb_sizes.y }, kernel_type, wavelet_info, double_buffering, optim_thread, optim_warp, warp_size, { (int)pairs_per_thread.x, (int)pairs_per_thread.y });

		float wavelet_type_cdf53_coef[] = { -0.5f, 0.25f };
		s_wavelet_type_info wavelet_type_cdf53 = { wavelet_type_cdf53_coef, 1.4142135623730f, 1, 1, std::string("WAVELET_TYPE_CDF53") };
		this->kernel_generator = new WaveletKernelGeneratorGpu({ (int)comb_sizes.x, (int)comb_sizes.y }, kernel_type, wavelet_info, double_buffering, optim_thread, optim_warp, warp_size, { (int)pairs_per_thread.x, (int)pairs_per_thread.y });
		//this->kernel_generator = new WaveletKernelGeneratorCpu<INTRINSICS_NONE, false>(kernel_type, &wavelet_type_cdf53, optim_thread, { 1,1 });
		//this->kernel_generator = new WaveletKernelGeneratorCpu<INTRINSICS_AVX512, true>(kernel_type, wavelet_info, optim_thread, { (int)pairs_per_thread.x, (int)pairs_per_thread.y });

		std::string step_body_str = kernel_generator->getBody();

		std::cerr << "op:       " << kernel_generator->op_count << std::endl
				  << "load:     " << kernel_generator->load_count << std::endl
				  << "store:    " << kernel_generator->store_count << std::endl
				  << "shuffle:  " << kernel_generator->shuffle_count << std::endl
				  << "barriers: " << kernel_generator->barrier_count << std::endl
				  << "steps:    " << kernel_generator->steps.size() << std::endl;


		this->gen_data = step_body_str;

		// delete whitespace characters
		size_t start_pos = 0;
		while ((start_pos = step_body_str.find(std::string(" "), start_pos)) != std::string::npos) {
			step_body_str.replace(start_pos, 1, "");
		}
		start_pos = 0;
		while ((start_pos = step_body_str.find(std::string("\n"), start_pos)) != std::string::npos) {
			step_body_str.replace(start_pos, 1, "");
		}

		str << std::string(" -D WAVELET_FILTER_BODY()=") + step_body_str;
	}
	return str.str();
}