#include "ParseArg.h"

ParseArg::ParseArg(unsigned long flags)
{
	this->flags = flags;
}

ParseArg::ParseArg(const std::vector<opt> &opts, unsigned long flags)
{
	this->flags = flags;
	this->addOpts(opts);
}


ParseArg::~ParseArg()
{
	this->clearOpts();
}

void ParseArg::addOpts(const std::vector<opt> &opts)
{
	for(int i = 0; i < opts.size(); i++)
		{
			this->addOpt(opts[i]);	
		}
}

void ParseArg::clearOpts()
{
	this->opts.clear();
}

void ParseArg::addOpt(const opt &i_opt)
{
	conv_opt opt;
	if(i_opt.long_par != NULL)
		{
			opt.long_par = i_opt.long_par;
			if(i_opt.value)
				{
					opt.lin_long = std::string() + "--" + i_opt.long_par + "=";
					opt.win_long = std::string() + "/" + i_opt.long_par + "=";
				}
			else
				{
					opt.lin_long = std::string() + "--" + i_opt.long_par;
					opt.win_long = std::string() + "/" + i_opt.long_par;
				}
		}
	if(i_opt.short_par != 0)
		{
			opt.short_par = i_opt.short_par;
			if(i_opt.value)
				{
					opt.lin_short = std::string() + "-" + i_opt.short_par + "=";
					opt.win_short = std::string() + "/" + i_opt.short_par + "=";
				}
			else
				{
					opt.lin_short = std::string() + "-" + i_opt.short_par;
					opt.win_short = std::string() + "/" + i_opt.short_par;
				}
		}
	opt.value = i_opt.value;
  opt.info = std::string() + i_opt.info;
  opt.id = i_opt.id;
	this->opts.push_back(opt);
}

void ParseArg::printHelp(std::string &header)
{
  // calculate maximum option length
  size_t max_opt_len = 0;
  for(unsigned int j = 0; j < this->opts.size(); j++)
    {
    size_t act_opt_len = 0;
      if(this->opts[j].long_par.length() != 0)
        {
          if(this->flags & PARSE_ARG_LINUX)
            {
              act_opt_len += this->opts[j].long_par.length() + 3;
            }
          if(this->flags & PARSE_ARG_WINDOWS)
            {
              act_opt_len += this->opts[j].long_par.length() + 2;
            }
        }
      if(this->opts[j].short_par.length() != 0)
        {
          if(this->flags & PARSE_ARG_LINUX)
            {
              act_opt_len += this->opts[j].short_par.length() + 2;
            }
          if(this->flags & PARSE_ARG_WINDOWS)
            {
              act_opt_len += this->opts[j].short_par.length() + 2;
            }
        }
      if(act_opt_len > max_opt_len)
        {
          max_opt_len = act_opt_len;
        }
    }

  std::stringstream format;
  format << " %-" << max_opt_len << "s %s\n";
  
  fprintf(stderr, "%s\n", header.c_str());
  for(unsigned int j = 0; j < this->opts.size(); j++)
    {
      std::string arg_text = "";
      if(this->opts[j].long_par.length() != 0)
        {
          if(this->flags & PARSE_ARG_LINUX)
            {
              arg_text += " --" + this->opts[j].long_par;
            }
          if(this->flags & PARSE_ARG_WINDOWS)
            {
              arg_text += " /" + this->opts[j].long_par;
            }
        }
      if(this->opts[j].short_par.length() != 0)
        {
          if(this->flags & PARSE_ARG_LINUX)
            {
              arg_text += " -" + this->opts[j].short_par;
            }
          if(this->flags & PARSE_ARG_WINDOWS)
            {
              arg_text += " /" + this->opts[j].short_par;
            }
        }
      fprintf(stderr, format.str().c_str(), arg_text.c_str(), this->opts[j].info.c_str());
    }
}


int ParseArg::getArgOpt(const std::string &arg, std::string &out_value)
{
	for(unsigned int j = 0; j < opts.size(); j++)
		{
			if(((this->opts[j].long_par.length() != 0) &&
						(((this->flags & PARSE_ARG_LINUX) && (this->compare(this->opts[j].lin_long, arg, this->opts[j].value, out_value))) ||
						((this->flags & PARSE_ARG_WINDOWS) && (this->compare(this->opts[j].win_long, arg, this->opts[j].value, out_value))))) ||
			   ((this->opts[j].short_par.length() != 0) &&
						(((this->flags & PARSE_ARG_LINUX) && (this->compare(this->opts[j].lin_short, arg, this->opts[j].value, out_value))) ||
						((this->flags & PARSE_ARG_WINDOWS) && (this->compare(this->opts[j].win_short, arg, this->opts[j].value, out_value))))))
				{
					return this->opts[j].id;
				}
		}
	return PARSE_ARG_NOT_FOUND;
}

bool ParseArg::compare(const std::string &opt, const std::string &arg, bool value, std::string &out_value)
{
	if(arg.compare(0, opt.length(), opt) != 0)
		{
			return false;
		}
	if(value)
		{
			out_value = arg.substr(opt.length());
		}
	return true;
}
