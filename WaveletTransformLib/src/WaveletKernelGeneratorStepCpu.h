#ifndef WaveletKernelGeneratorStepCpu_H
#define WaveletKernelGeneratorStepCpu_H
#include "WaveletKernelGeneratorStep.h"
#include "WaveletKernelGeneratorArg.h"
#include <map>


template <e_intrinsics intrinsics>
inline std::string getFloatIntrinsicsType()
{
	return "float";
}

template <>
inline std::string getFloatIntrinsicsType<INTRINSICS_SSE>()
{
	return "__m128";
}

template <>
inline std::string getFloatIntrinsicsType<INTRINSICS_AVX>()
{
	return "__m256";
}

template <>
inline std::string getFloatIntrinsicsType<INTRINSICS_AVX512>()
{
	return "__m512";
}

template <e_intrinsics intrinsics>
inline std::string getFloatIntrinsicsFunc()
{
	return "_scalar";
}

template <>
inline std::string getFloatIntrinsicsFunc<INTRINSICS_SSE>()
{
	return "_mm";
}

template <>
inline std::string getFloatIntrinsicsFunc<INTRINSICS_AVX>()
{
	return "_mm256";
}

template <>
inline std::string getFloatIntrinsicsFunc<INTRINSICS_AVX512>()
{
	return "_mm512";
}


template <e_intrinsics intrinsics, bool fma_enabled>
class WaveletKernelGeneratorStepCpu: public WaveletKernelGeneratorStep
{
public:
	WaveletKernelGeneratorStepCpu(s_wavelet_type_info *wavelet_info, std::array<int, 2> pairs_per_thread, bool optim_thread, std::vector<std::array<unsigned int, 4>> input_memory_locations, std::string function_name, WaveletKernelGeneratorStep *parent = NULL, bool write_all_subbands = false);
	std::string add(const std::string &a, const std::string &b);
	std::string fma(const std::string &a, const std::string &b, const std::string &c);
	std::string mul(const std::string &a, const std::string &b);
	std::string set(const std::string &out, const std::string &value) const;

	std::string getValue(std::string reg, std::array<unsigned char, 4> coef_local_id, std::array<int, 2> pos, std::array<int, 2> reg_pos, e_subband subband_id, bool is_read);
	std::string getFloat(float val);
	std::string getBody() const;
	bool isEmpty() const;

	std::map<std::string, WaveletKernelGeneratorArg> input_args;
	std::map<std::string, WaveletKernelGeneratorArg> output_args;

	std::vector<std::array<unsigned int, 4>> input_memory_locations;
	std::vector<std::array<unsigned int, 4>> output_memory_locations;
	std::string getHeader() const;
	std::string getStepHeader(const WaveletKernelGeneratorStepCpu &step) const;

private:
	void appendStep();
	unsigned int findFreeMemoryLocation();
	const WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *getCpuStepAt(size_t id) const;
	const WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *getCpuStepBack() const;

	std::string getInputAssign() const;
	
	std::vector<std::array<bool, 4>> is_read;
	std::vector<std::array<bool, 4>> is_write;

};


template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::set(const std::string &out, const std::string &value) const
{
	return out + " = " + value + ";\n";
}

template <e_intrinsics intrinsics, bool fma_enabled>
WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::WaveletKernelGeneratorStepCpu(s_wavelet_type_info *wavelet_info, std::array<int, 2>  pairs_per_thread, bool optim_thread, std::vector<std::array<unsigned int, 4>> input_memory_locations, std::string function_name, WaveletKernelGeneratorStep *parent, bool write_all_subbands) : WaveletKernelGeneratorStep(wavelet_info, pairs_per_thread, optim_thread, false, function_name, parent)
{
	this->input_memory_locations = input_memory_locations;
	this->output_memory_locations = input_memory_locations;
	std::array<bool, 4> channels = { false, false, false, false };
	is_read = std::vector<std::array<bool, 4>>(pairs_per_thread[0] * pairs_per_thread[1], channels);
	is_write = std::vector<std::array<bool, 4>>(pairs_per_thread[0] * pairs_per_thread[1], channels);
	if(write_all_subbands)
	{
		for(int x = 0; x < pairs_per_thread[0]; x++)
			for (int y = 0; y < pairs_per_thread[1]; y++)
				for (int ch = 0; ch < 4; ch++)
				{
					is_read[x + y * pairs_per_thread[0]][ch] = true;
					getValue(std::string(), empty_coef, { 0,0 }, { x,y }, (e_subband)ch, false);
					getValue(std::string(), empty_coef, { 0,0 }, { x,y }, (e_subband)ch, true);
				}
	}
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_NONE, true>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count++;
	return getFloatIntrinsicsFunc<INTRINSICS_NONE>() + "_fmadd_ps(" + a + "," + b + "," + c + ")";
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_SSE, true>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count++;
	return getFloatIntrinsicsFunc<INTRINSICS_SSE>() + "_fmadd_ps(" + a + "," + b + "," + c + ")";
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_AVX, true>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count++;
	return getFloatIntrinsicsFunc<INTRINSICS_AVX>() + "_fmadd_ps(" + a + "," + b + "," + c + ")";
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_AVX512, true>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count++;
	return getFloatIntrinsicsFunc<INTRINSICS_AVX512>() + "_fmadd_ps(" + a + "," + b + "," + c + ")";
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_NONE, false>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count += 2;
	return add(mul(a, b), c);
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_SSE, false>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count += 2;
	return add(mul(a, b), c);
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_AVX, false>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count += 2;
	return add(mul(a, b), c);
}

template <>
inline std::string WaveletKernelGeneratorStepCpu<INTRINSICS_AVX512, false>::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count += 2;
	return add(mul(a, b), c);
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::mul(const std::string &a, const std::string &b)
{
	this->op_count++;
	return getFloatIntrinsicsFunc<intrinsics>() + "_mul_ps(" + a + "," + b + ")";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::add(const std::string &a, const std::string &b)
{
	this->op_count++;
	return getFloatIntrinsicsFunc<intrinsics>() + "_add_ps(" + a + "," + b + ")";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getFloat(float val)
{
	return  getFloatIntrinsicsFunc<intrinsics>() + "_set1_ps(" + std::to_string(val) + ")";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getValue(std::string reg, std::array<unsigned char, 4> coef_local_id, std::array<int, 2> pos, std::array<int, 2> reg_pos, e_subband subband_id, bool is_read)
{
	if (parent != NULL) this->parent->getValue(reg, coef_local_id, pos, reg_pos, subband_id, is_read);

	unsigned int reg_pos_id = reg_pos[0] + reg_pos[1] * pairs_per_thread[0];
	WaveletKernelGeneratorArg src_arg(pos, reg_pos, subband_id, this->input_memory_locations[reg_pos_id][subband_id]);
	std::string printable_arg_name = src_arg.getName(!is_read);
	std::string src_arg_name = src_arg.getName();
	if (!is_read)
	{
		std::map<std::string, WaveletKernelGeneratorArg>::iterator arg_it = output_args.find(src_arg_name);
		if (arg_it == output_args.end())
		{
			WaveletKernelGeneratorArg write_src_arg = src_arg;
			if(this->is_read[reg_pos_id][subband_id])
			{
				this->output_memory_locations[reg_pos_id][subband_id] = findFreeMemoryLocation();
				write_src_arg.memory_block = this->output_memory_locations[reg_pos_id][subband_id];
			}
			output_args.insert(std::map< std::string, WaveletKernelGeneratorArg >::value_type(src_arg_name, write_src_arg));
			this->is_write[reg_pos_id][subband_id] = true;
		}
	}
	//else
	//{
		std::map<std::string, WaveletKernelGeneratorArg>::iterator arg_it = input_args.find(src_arg_name);
		if ((pos[0] != 0) || (pos[1] != 0))
		{
			if(this->is_write[reg_pos_id][subband_id] && (!this->is_read[reg_pos_id][subband_id]))
			{
				WaveletKernelGeneratorArg write_src_arg = src_arg.toOutputArg();
				std::map<std::string, WaveletKernelGeneratorArg>::iterator write_arg_it = output_args.find(write_src_arg.getName());
				if (write_arg_it != output_args.end())
				{
					this->output_memory_locations[reg_pos_id][subband_id] = findFreeMemoryLocation();
					write_arg_it->second.memory_block = this->output_memory_locations[reg_pos_id][subband_id];
				}
			}
			this->is_read[reg_pos_id][subband_id] = true;
		}
		if (arg_it == input_args.end()) input_args.insert(std::map< std::string, WaveletKernelGeneratorArg >::value_type(src_arg_name, src_arg));
	//}
	return printable_arg_name;
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getBody() const
{
	std::string out;
	for(int i = 0; i < this->steps.size(); i++)
	{
		out += this->steps.at(i)->getBody();
	}
	return out + "\nstatic void " + getHeader() + "{\n" + getInputAssign() + WaveletKernelGeneratorStep::getBody() + "}\n";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getHeader() const
{
	std::string out;
	std::map<std::string, WaveletKernelGeneratorArg>::const_iterator args_iterator;
	for (args_iterator = output_args.begin(); args_iterator != output_args.end(); args_iterator++)
	{
		if (args_iterator != output_args.begin()) out += ", ";
		out += getFloatIntrinsicsType<intrinsics>() + " " + args_iterator->second.getName(true);
	}
	for(args_iterator = input_args.begin(); args_iterator != input_args.end(); args_iterator++)
	{
		if((args_iterator != input_args.begin()) || (output_args.size() != 0)) out += ", ";
		out += getFloatIntrinsicsType<intrinsics>() + " " + args_iterator->second.getName(false);
	}
	return function_name + "(" + out + ")\n";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getStepHeader(const WaveletKernelGeneratorStepCpu &step) const
{
	std::string out;
	std::map<std::string, WaveletKernelGeneratorArg>::const_iterator args_iterator;
	for (args_iterator = step.output_args.begin(); args_iterator != step.output_args.end(); args_iterator++)
	{
		if (args_iterator != step.output_args.begin()) out += ", ";
		out += args_iterator->second.getName(true, true);
	}
	for (args_iterator = step.input_args.begin(); args_iterator != step.input_args.end(); args_iterator++)
	{
		if ((args_iterator != step.input_args.begin()) || (step.output_args.size() != 0)) out += ", ";
		std::map<std::string, WaveletKernelGeneratorArg>::const_iterator arg_it = output_args.find(args_iterator->second.getName());
		out += args_iterator->second.getName(false, arg_it != output_args.end());
	}
	return step.function_name + "(" + out + ");\n";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getInputAssign() const
{
	std::ostringstream out;
	std::map<std::string, WaveletKernelGeneratorArg>::const_iterator args_iterator;
	for (args_iterator = output_args.begin(); args_iterator != output_args.end(); args_iterator++)
	{
		out << this->set(args_iterator->second.getName(true),args_iterator->second.getName(false));
	}

	for (int i = 0; i < this->steps.size(); i++)
	{
		out << this->getStepHeader(*this->getCpuStepAt(i));
	}
	return out.str();
}

template <e_intrinsics intrinsics, bool fma_enabled>
unsigned WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::findFreeMemoryLocation()
{
	for(unsigned int mem_location = 0; mem_location < this->input_memory_locations.size() * this->input_memory_locations[0].size() * 2; mem_location++)
	{
		bool mem_location_used = false;
		for(unsigned int subband = 0; subband < input_memory_locations[0].size(); subband++)
		{
			for (unsigned int reg_pos_id = 0; reg_pos_id < this->input_memory_locations.size(); reg_pos_id++)
			{
				if ((input_memory_locations[reg_pos_id][subband] == mem_location) || (output_memory_locations[reg_pos_id][subband] == mem_location))
				{
					mem_location_used = true;
					break;
				}
			}
		}
		if (!mem_location_used) return mem_location;
	}
	std::cerr << "Error: internal error memory location out of range." << std::endl;
	exit(10);
}


template <e_intrinsics intrinsics, bool fma_enabled>
void WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::appendStep()
{
	std::vector<std::array<unsigned int, 4>> memory_locations;
	if (steps.size() == 0)
	{
#ifdef WRITE_ALL_OUTPUTS_IN_STEP_1
		memory_locations = std::vector<std::array<unsigned int, 4>>(pairs_per_thread[0] * pairs_per_thread[1], { std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max() });
#else
		memory_locations = std::vector<std::array<unsigned int, 4>>(pairs_per_thread[0] * pairs_per_thread[1], { 0,1,2,3 });
#endif
	}
	else
	{
		if (((WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *)(&*steps.back()))->op_count == 0) return;
		memory_locations = ((WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *)(&*steps.back()))->output_memory_locations;
	}
	steps.push_back(std::shared_ptr<WaveletKernelGeneratorStep>(new WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>(this->wavelet_info, pairs_per_thread, optim_thread, memory_locations, this->function_name + "_" + std::to_string(steps.size()), this)));
}

template <e_intrinsics intrinsics, bool fma_enabled>
const WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getCpuStepAt(size_t id) const
{
	if (id >= this->steps.size()) return NULL;
	return (WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *)(&*this->steps.at(id));
}

template <e_intrinsics intrinsics, bool fma_enabled>
const WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::getCpuStepBack() const
{
	return (WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *)(&*this->steps.back());
}


template <e_intrinsics intrinsics, bool fma_enabled>
bool WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>::isEmpty() const
{
	bool is_empty = this->op_count == 0;
	for(int i = 0; i < steps.size(); i++)
	{
		is_empty = is_empty && (steps[i]->op_count == 0);
	}
	return is_empty;
}



#endif
