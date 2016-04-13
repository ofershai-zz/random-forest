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


#include "ExecutionConfiguration.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

std::tr1::unordered_map<std::string, std::string> ExecutionConfiguration::_strings;
std::tr1::unordered_map<std::string, int> ExecutionConfiguration::_ints;
std::tr1::unordered_map<std::string, double> ExecutionConfiguration::_doubles;

void ExecutionConfiguration::parseConfiguration(const std::string &path) {
    std::ifstream conf(path.c_str());
    if (conf.fail()) {
        std::cerr << "could not open file " << path << std::endl;
        return;
    }

    while (!conf.fail() && !conf.eof()) {
        std::string property, type, value;
        char peek;
        conf >> peek;
        if (peek == '#') {
            char line[2048];
            conf.getline(line, 2048, '\n');
        } else {
            conf >> property >> type >> value;
            if (peek != '<' || property.compare(0, 9, "property=") != 0
                    || type.compare(0, 5, "type=") != 0 || type[type.size() - 1] != '>') {
                if (!conf.eof())
                        std::cerr << "Malformed property line: " << peek << property << " "
                                << type << " " << value << std::endl;
            } else {
                if (!type.compare(5, 6, "string")) {
                    _strings[property.substr(9, property.size() - 9)] = value;
                } else if (!type.compare(5, 6, "double")) {
                    _doubles[property.substr(9, property.size() - 9)] = atof(value.c_str());
                } else if (!type.compare(5, 3, "int")) {
                    _ints[property.substr(9, property.size() - 9)] = atoi(value.c_str());
                } else {
                    std::cerr << "invalid type: " << type.substr(5, type.size() - 6) << std::endl;
                }
            }
        }
    }
}

void ExecutionConfiguration::printConfiguration(std::ostream &out) {
    for (std::tr1::unordered_map<std::string, std::string>::iterator i = _strings.begin(); i != _strings.end(); ++i) {
        out << "<property=" << i->first << " " << "type=string> " << i->second << std::endl;
    }
    for (std::tr1::unordered_map<std::string, int>::iterator i = _ints.begin(); i != _ints.end(); ++i) {
        out << "<property=" << i->first << " " << "type=int> " << i->second << std::endl;
    }
    for (std::tr1::unordered_map<std::string, double>::iterator i = _doubles.begin(); i != _doubles.end(); ++i) {
        out << "<property=" << i->first << " " << "type=double> " << i->second << std::endl;
    }
}

