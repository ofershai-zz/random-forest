/*
 * Copyright (C) 2014 Ofer Shai
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
