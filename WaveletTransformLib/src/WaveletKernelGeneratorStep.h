#ifndef WaveletKernelGeneratorStep_H
#define WaveletKernelGeneratorStep_H
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <cfloat>
#include "WaveletKernelGeneratorArg.h"
#include "WaveletKernelGeneratorTypes.h"
#include "WaveletKernelGeneratorHelper.h"

//extern const std::string empty_string;
extern const std::string orig_reg_name_f4;
extern const std::string temp_reg_name_f1;
extern const std::string temp_reg_name_f4;
extern std::array<unsigned char, 4> empty_coef;
extern const std::vector<std::string> subband_labels;

extern std::array<float, 4> operator-(std::array<float, 4> &input);

struct s_step_borders
{
	std::array<int, 2> min_b[4];
	std::array<int, 2> max_b[4];
	std::array<unsigned char, 4> save_element_id;
	std::array<unsigned char, 4> act_pos_id;

	s_step_borders(std::array<unsigned char, 4> save_element_id, std::array<unsigned char, 4> act_pos_id)
	{
		clear(save_element_id, act_pos_id);
	}
	void resize(std::array<int, 2> point, int element_id)
	{
		if (min_b[element_id][0] == 255)
		{
			min_b[element_id] = point;
			max_b[element_id] = point;
		}
		else
		{
			min_b[element_id][0] = std::min(point[0], min_b[element_id][0]);
			min_b[element_id][1] = std::min(point[1], min_b[element_id][1]);
			max_b[element_id][0] = std::max(point[0], max_b[element_id][0]);
			max_b[element_id][1] = std::max(point[1], max_b[element_id][1]);
		}
	}
	void clear(std::array<unsigned char, 4> save_element_id, std::array<unsigned char, 4> act_pos_id)
	{
		for (int i = 0; i < 4; i++)
		{
			min_b[i] = { 255,255 };
			max_b[i] = { 255,255 };
		}
		this->act_pos_id = act_pos_id;
		this->save_element_id = save_element_id;
	}
	std::string toString()
	{
		std::ostringstream out("");
		for (int i = 0; i < 4; i++)
		{
			out << (save_element_id[i] == 255 ? 255 : 0) << ":" << (int)act_pos_id[i] << std::endl;
		}
		out << std::endl;
		for (int i = 0; i < 4; i++)
		{
			out << subband_labels[i] << ":x(" << min_b[i][0] << "," << max_b[i][0] << "),y(" << min_b[i][1] << "," << max_b[i][1] << ")" << std::endl;
		}
		return out.str();
	}
};

struct s_local_borders
{
	std::array<int, 2> min_b;
	std::array<int, 2> max_b;
	int min_pos_id;
	int max_pos_id;
	s_local_borders()
	{
		clear();
	}
	void resize(s_local_borders in_local_borders)
	{
		this->resize(in_local_borders.min_b, in_local_borders.min_pos_id);
		this->resize(in_local_borders.max_b, in_local_borders.max_pos_id);
	}
	void resize(std::array<int, 2> point, int pos_id)
	{
		if (pos_id < min_pos_id)
		{
			min_b = { 0,0 };
			min_pos_id = pos_id;
		}
		if (pos_id == min_pos_id)
		{
			if (point[1] < min_b[1])
			{
				min_b = point;
			}
			else if (point[1] == min_b[1])
			{
				min_b[0] = std::min(point[0], min_b[0]);
			}
		}
		if (pos_id > max_pos_id)
		{
			max_b[0] = 0;
			max_b[1] = 0;
			max_pos_id = pos_id;
		}
		if (pos_id == max_pos_id)
		{
			if (point[1] > max_b[1])
			{
				max_b = point;
			}
			else if (point[1] == max_b[1])
			{
				max_b[0] = std::max(point[0], max_b[0]);
			}
		}
	}

	void clear()
	{
		min_b = { 0,0 };
		min_pos_id = 0;
		max_b = { 0,0 };
		max_pos_id = 0;
	}
	std::string toString()
	{
		std::ostringstream out("");
		out << min_pos_id << ":" << min_b[0] << "," << min_b[1] << std::endl << max_pos_id << ":" << max_b[0] << "," << max_b[1] << std::endl;
		return out.str();
	}
};

struct v_step_borders
{
	std::vector<s_step_borders> elements;
	v_step_borders()
	{
		clear();
	}
	void resize(std::array<int, 2> point, int element_id)
	{
		elements.back().resize(point, element_id);
	}

	void add_step(std::array<unsigned char, 4> save_element_id, std::array<unsigned char, 4> act_pos_id)
	{
		elements.push_back(s_step_borders(save_element_id, act_pos_id));
	}

	void clear()
	{
		elements.clear();
		elements.push_back(s_step_borders(empty_coef, empty_coef));
	}
	std::string toString()
	{
		std::ostringstream out("");
		for (std::vector<s_step_borders>::iterator it = elements.begin(); it != elements.end(); it++)
		{
			out << it->toString() << std::endl;
		}
		return out.str();
	}
};

class WaveletKernelGeneratorStep
{
public:
	WaveletKernelGeneratorStep(s_wavelet_type_info *wavelet_info, std::array<int, 2> pairs_per_thread, bool optim_thread, bool read_once, std::string function_name, WaveletKernelGeneratorStep *parent = NULL);
	virtual std::string add(const std::string &a, const std::string &b) = 0;
	virtual std::string fma(const std::string &a, const std::string &b, const std::string &c) = 0;
	virtual std::string mul(const std::string &a, const std::string &b) = 0;
	virtual std::string set(const std::string &out, const std::string &value) const = 0;

	virtual std::string getValue(std::string reg, std::array<unsigned char, 4> coef_local_id, std::array<int, 2> pos, std::array<int, 2> reg_pos, e_subband subband_id, bool is_read) = 0;
	virtual std::string getFloat(float val) = 0;

	virtual std::string getBody() const = 0;

	void appendSubstepNormBody();
	
	void appendNormBody();
	
	void appendSubstep2DBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	
	void append2DBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);

	void appendSubsteps1DHorizontalBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void appendSubsteps1DVerticalBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);

	void appendSubstep1DLeftBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void appendSubstep1DRightBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void appendSubstep1DTopBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void appendSubstep1DBottomBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);

	void append1DLeftBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void append1DRightBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void append1DTopBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);
	void append1DBottomBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4);

	void appendSubstep1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg = orig_reg_name_f4);
	void appendSubstep1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &l_coef, std::vector<float> &h_coef, const std::string &src_reg = orig_reg_name_f4, const std::string &tmp_reg = temp_reg_name_f1);
	void appendSubstep1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg = orig_reg_name_f4);
	void appendSubstep1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &h_coef, std::vector<float> &l_coef, const std::string &src_reg = orig_reg_name_f4, const std::string &tmp_reg = temp_reg_name_f1);
	void appendSubstep2DGenBody(int half_filter_size, std::array<unsigned char, 4> coef_local_id, std::vector<float> &ll_coef, std::vector<float> &hl_coef, std::vector<float> &lh_coef, std::vector<float> &hh_coef, const std::string &src_reg, const std::string &tmp_data);

	void append1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg = orig_reg_name_f4);
	void append1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &l_coef, std::vector<float> &h_coef, const std::string &src_reg = orig_reg_name_f4, const std::string &tmp_reg = temp_reg_name_f1);
	void append1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg = orig_reg_name_f4);
	void append1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &h_coef, std::vector<float> &l_coef, const std::string &src_reg = orig_reg_name_f4, const std::string &tmp_reg = temp_reg_name_f1);
	void append2DGenBody(int half_filter_size, std::array<unsigned char, 4> coef_local_id, std::vector<float> &ll_coef, std::vector<float> &hl_coef, std::vector<float> &lh_coef, std::vector<float> &hh_coef, const std::string &src_reg, const std::string &tmp_data);

	void appendSubstepsImprovedUpdateBody(unsigned int iter_id);
	void appendSubstepImprovedUpdateBodyH(unsigned int iter_id);
	void appendSubstepImprovedUpdateBodyV(unsigned int iter_id);
	void appendSubstepsImprovedPredictBody(unsigned int iter_id);
	void appendSubstepImprovedPredictBodyH(unsigned int iter_id);
	void appendSubstepImprovedPredictBodyV(unsigned int iter_id);

	void appendImprovedUpdateBodyH(unsigned int iter_id);
	void appendImprovedUpdateBodyV(unsigned int iter_id);
	void appendImprovedPredictBodyH(unsigned int iter_id);
	void appendImprovedPredictBodyV(unsigned int iter_id);

	void appendLoadFromLocalBody(std::array<unsigned char, 4> coef_local_id, e_subband src_id, e_subband dst_id, int i, int j, float coef, unsigned char op, const std::string &src_reg = orig_reg_name_f4, const std::string &dst_reg = orig_reg_name_f4);
	void appendLoadFromLocalBody(std::array<unsigned char, 4> coef_local_id, e_subband src_id, int i, int j, std::array<float, 4> coefs, unsigned char op, const std::string &src_reg = orig_reg_name_f4, const std::string &dst_reg = orig_reg_name_f4, const std::string &tmp_reg = temp_reg_name_f1);

	void appendToBody(std::string &val);
	void appendToSubstepBody(std::string &val);

	s_local_borders local_borders;
	v_step_borders step_borders;
	int op_count;
	int shuffle_count;
	int store_count;
	int load_count;
	int barrier_count;
	bool optim_thread;

	s_local_borders getLocalBorders() const;
	v_step_borders getStepBorders() const;
	int getOpCount() const;
	int getShuffleCount() const;
	int getStoreCount() const;
	int getLoadCount() const;
	int getBarrierCount() const;

	std::string function_name;
protected:

	virtual void appendStep() = 0;

	std::vector<std::shared_ptr<WaveletKernelGeneratorStep>> steps;
	bool read_once;

	std::ostringstream body;

	std::array<int, 2> pairs_per_thread;
	s_wavelet_type_info *wavelet_info;
	WaveletKernelGeneratorStep *parent;
};

#endif
