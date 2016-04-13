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

#ifndef DATASUBSET_H
#define	DATASUBSET_H

#include "Data.h"
#include <vector>
#include <iostream>

class DataSubsetIterator {
    friend class DenseData;
    friend class SparseData;
private:
    std::vector<int> *_rows;
    std::vector<double> *_vals;
    std::vector<int> _indeces;
    int _idx;
    int _size;

public:
    DataSubsetIterator(DataSubset &ds, int feature);

    size_t size() {
        return _size;
    }

    bool hasNext() {
        return _idx < _size;
    }

    void next() {
        _idx++;
    }

    int index() {
        if (!_rows) 
            return _indeces[_idx];
        
        return _rows->at(_indeces[_idx]);
    }

    double value() {
        return _vals->at(_indeces[_idx]);
    }

    void reset() {
        _idx = 0;
    }
        
};

class DataSubset {
public:

private:
    Data *_data;
    std::vector<int> _userIndeces;
    DataSubset(Data *data, const std::vector<int> &indeces);

public:

    DataSubset(Data *data);

    std::vector<int> getSomeFeatures(int nFeatures);

    const std::vector<int> &getUsers() {
        return _userIndeces;
    }

    static void permute(std::vector<int> &tmp);

    int classificationY(int u) {
        return _data->_classificationY[u];
    }

    double regressionY(int u) {
        return _data->_regressionY[u];
    }

    int nClasses() {
        return _data->_nClasses;
    }

    int nUsers() {
        return _userIndeces.size();
    }

    int nFeatures() {
        return _data->_nFeatures;
    }

    bool at(int user, int feature, double &v) {
        return _data->at(user, feature, v);
    }

    // creates a subset of all features for selected users

    DataSubset createSubsetUsers(const std::vector<int> &users) {
        return DataSubset(_data, users);
    }
    
    friend class DataSubsetIterator;
    
};

#endif	/* DATASUBSET_H */

