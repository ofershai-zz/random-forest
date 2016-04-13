/*
 * ExecutionConfiguration.h
 *
 *  Created on: 2013-01-14
 *      Author: ofer
 */

#ifndef EXECUTIONCONFIGURATION_H_
#define EXECUTIONCONFIGURATION_H_

#include <tr1/unordered_map>
#include <string>
#include <stdexcept>

class ExecutionConfiguration
    {
    private:
	static std::tr1::unordered_map<std::string, std::string> _strings;
	static std::tr1::unordered_map<std::string, int> _ints;
	static std::tr1::unordered_map<std::string, double> _doubles;

    public:

	static std::string &getString(std::string prop) { 
            if (_strings.find(prop) == _strings.end())
                throw std::range_error("invalid string property " + prop);
            return _strings[prop]; 
        }
	static bool stringExists(std::string prop) { return _strings.find(prop) != _strings.end(); }
	static int getInt(std::string prop) { 
            if (_ints.find(prop) == _ints.end())
                throw std::range_error("invalid int property " + prop);
            return _ints[prop]; 
        }
	static bool intExists(std::string prop) { return _ints.find(prop) != _ints.end(); }
	static double getDouble(std::string prop) { 
            if (_doubles.find(prop) == _doubles.end())
                throw std::range_error("invalid double property " + prop);
            return _doubles[prop]; 
        }
	static bool doubleExists(std::string prop) { return _doubles.find(prop) != _doubles.end(); }

	static void parseConfiguration(const std::string &path);
	static void printConfiguration(std::ostream &out);

    };

#endif /* EXECUTIONCONFIGURATION_H_ */
