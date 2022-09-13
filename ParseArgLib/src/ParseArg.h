#ifndef PARSE_ARG_H
#define PARSE_ARG_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>

#define PARSE_ARG_LINUX					0x00000001 ///< parse only linux prefix arguments
#define PARSE_ARG_WINDOWS				0x00000002 ///< parse only windows prefix arguments
#define PARSE_ARG_ALL           0xFFFFFFFF ///< parse all prefix arguments

#define PARSE_ARG_NOT_FOUND							-1 ///< undefined option

/**
 * @brief Parameter option struct
 */
typedef struct
	{
    unsigned int id;
		const char *long_par; ///< long version of parameter, NULL if not use
		char short_par; ///< one character parameter, 0 if not use
		bool value; ///< true if returning a value
    const char *info; ///< help
	}opt;

/**
 * @brief Internal converted parameter option struct
 */
typedef struct
	{
    unsigned int id; ///< id of value
		std::string long_par; ///< long parameter without os prefix
		std::string short_par; ///< one character parameter without os prefix
		std::string lin_long; ///< long parameter with linux os prefix
		std::string win_long; ///< long parameter with windows os prefix
		std::string lin_short; ///< one character parameter with linux prefix
		std::string win_short; ///< one character parameter with windows prefix
		bool value; ///< true if returning a value
    std::string info; ///< argument info value
	}conv_opt;

typedef std::vector<conv_opt> v_opt;

/**
 * @brief Class for parsing arguments
 */
class ParseArg
{
	public:
    /**
     * Init object and set type of arguments.
     * @param flags type of arguments
     */
		ParseArg(unsigned long flags = PARSE_ARG_ALL);
    /**
     * Init object and set options of arguments.
     * @param opts options
     * @param opt_len options count
     * @param flags type of arguments
     */
		ParseArg(const std::vector<opt> &opts, unsigned long flags = PARSE_ARG_ALL);
    /**
     * Delete object data.
     */
		~ParseArg();
    /**
     * Print help.
     * @param header Header of help
     */
		void printHelp(std::string &header);
    /**
     * Get index of processing argument.
     * @param arg processing argument
     * @param out_value argument data
     */
		int getArgOpt(const std::string &arg, std::string &out_value);
    /**
     * Add options of arguments.
     * @param opts options
     * @param opt_len options count
     */
		void addOpts(const std::vector<opt> &opts);
    /**
     * Add option of arguments.
     * @param i_opt option
     */
		void addOpt(const opt &i_opt);
    /**
     * Clear options of arguments.
     */
		void clearOpts();
    /**
     * Set type of arguments.
     * @param flags type of arguments
     */
		void setFlags(int flags = PARSE_ARG_ALL);
	private:
    /**
     * Check if option "opt" is prefix of argument "arg".
     * @param opt processing option
     * @param arg processing argument
     * @param value option is returning a value
     * @param out_value argument value
     * @return option "opt" is prefix of argument "arg"
     */
		bool compare(const std::string &opt, const std::string &arg, bool value, std::string &out_value);
		unsigned long flags; ///< type of arguments
		v_opt opts; ///< vector of internal structure options
};

#endif

