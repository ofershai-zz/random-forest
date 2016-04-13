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


#include "DataSubset.h"
#include "Data.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <boost/random.hpp>
#include <ctime>

DataSubsetIterator::DataSubsetIterator(DataSubset &ds, int feature) {
    ds._data->iterator(this, feature, ds._userIndeces);
}

/*
DataSubsetIterator::DataSubsetIterator(std::vector<int> &rows, std::vector<double> &vals, std::vector<int> &indeces, int start, int end) :
        _rows(&rows), _vals(&vals), _idx(0), _size(0) {   
    size_t i = 0;
    int idx = start;
    while (i < indeces.size() && idx < end) {
        while (idx < end && indeces[i] > _rows->at(idx))
            idx++;
        if (idx < end) {
            while (i < indeces.size() && indeces[i] < _rows->at(idx))
                i++;
            while (i < indeces.size() && indeces[i] == _rows->at(idx)) {
                _indeces.push_back(idx);
                _size++;
                i++;
            }
        }
    }    
}

DataSubset::Iterator DataSubset::featureIterator(int f) {
    return Iterator(_data->_userT,
            _data->_valT,
            _userIndeces,
            f <= 0 ? 0 : _data->_featureT[f - 1],
            _data->_featureT[f]);
}
 
 */

DataSubset::DataSubset(Data *data) {
    _data = data;
    _userIndeces.reserve(data->_nUsers);
    for (size_t i = 0; i < data->_nUsers; i++) 
        _userIndeces.push_back(i);
}

DataSubset::DataSubset(Data *data, const std::vector<int> &indeces) {
    _data = data;
    std::vector<int> tmp(indeces);
    _userIndeces.swap(tmp);
    std::sort(_userIndeces.begin(), _userIndeces.end());        
}

std::vector<int> DataSubset::getSomeFeatures(int nFeatures) {
    static boost::mt19937 rng(std::time(0));
    
    std::vector<int> tmp(_data->_featureList);
    DataSubset::permute(tmp);
    
    tmp.resize(nFeatures);
    return tmp;    
}

void DataSubset::permute(std::vector<int> &tmp) {
    static boost::mt19937 rng(std::time(0));
    
    for (size_t i = 0; i < tmp.size(); i++)
        tmp[i] = i;
    for (size_t i = 0; i < tmp.size(); i++) {
        boost::uniform_int<> swap(i, tmp.size()-1);
        int r = swap(rng);   
        int t = tmp[i];
        tmp[i] = tmp[r];
        tmp[r] = t;
    }
}
