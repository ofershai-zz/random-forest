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


#include "mmio.h"
#include "Data.h"
#include "DataSubset.h"
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <set>
#include <algorithm>

void Data::filterFeatures(std::string filename) {
    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        std::cerr << "could not open file " << filename << std::endl;
        exit(1);
    }
    MM_typecode typecode;

    int rc = mm_read_banner(file, &typecode);
    if (rc != 0) {
        std::string err;
        switch (rc) {
            case MM_PREMATURE_EOF:
                err = "Premature EOF at header";
                break;
            case MM_NO_HEADER:
                err = "Missing Header";
                break;
            case MM_UNSUPPORTED_TYPE:
                err = "Unrecognized Header";
                break;
            default:
                err = "Unexpected return value from mm_read_banner";
        }
        throw std::runtime_error (err);
    }

    if (!mm_is_array(typecode) || !mm_is_integer(typecode) || !mm_is_general(typecode))
        throw std::runtime_error ("Unsupported Matrix Market file. Currently, only array integer general files are supported for feature filter");

    int M, N;
    rc = mm_read_mtx_array_size(file, &M, &N);
    if (rc != 0) {
        std::string err;
        switch (rc) {
            case MM_PREMATURE_EOF:
                err = "Premature EOF at size info";
                break;
            default:
                err = "Unexpected return value from mm_read_mtx_crd_size";
        }
        throw std::runtime_error (err);
    } else if (M > (int) _nFeatures || N != 1) {
        throw std::runtime_error ("Feature filter size does not match features. It must be rows X 1, and rows must be less than or equal number of features");
    }

    _featureList.resize(M);
    
    for (size_t idx = 0; idx < M; idx++) {
        int v;
        rc = fscanf(file, "%d", &v);
        if (rc < 1)
            throw std::runtime_error ("Unexpected error while reading the input file");
        if (v < 0) 
            throw std::runtime_error ("target mast be a non-negative integer");
        
        _featureList[idx] = v;
    }
}

void Data::loadClassificationY(std::string filename) {
    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        std::cerr << "could not open file " << filename << std::endl;
        exit(1);
    }
    MM_typecode typecode;

    int rc = mm_read_banner(file, &typecode);
    if (rc != 0) {
        std::string err;
        switch (rc) {
            case MM_PREMATURE_EOF:
                err = "Premature EOF at header";
                break;
            case MM_NO_HEADER:
                err = "Missing Header";
                break;
            case MM_UNSUPPORTED_TYPE:
                err = "Unrecognized Header";
                break;
            default:
                err = "Unexpected return value from mm_read_banner";
        }
        throw std::runtime_error (err);
    }

    if (!mm_is_array(typecode) || !mm_is_integer(typecode) || !mm_is_general(typecode))
        throw std::runtime_error ("Unsupported Matrix Market file. Currently, only array integer general files are supported for classification targets");

    int M, N;
    rc = mm_read_mtx_array_size(file, &M, &N);
    if (rc != 0) {
        std::string err;
        switch (rc) {
            case MM_PREMATURE_EOF:
                err = "Premature EOF at size info";
                break;
            default:
                err = "Unexpected return value from mm_read_mtx_crd_size";
        }
        throw std::runtime_error (err);
    } else if (M != (int) _nUsers || N != 1) {
        throw std::runtime_error ("Target size does not match features. It must be rows X 1");
    }

    _classificationY.resize(_nUsers);
    std::set<int> classes;
    
    for (size_t idx = 0; idx < _nUsers; idx++) {
        int v;
        rc = fscanf(file, "%d", &v);
        if (rc < 1)
            throw std::runtime_error ("Unexpected error while reading the input file");
        if (v < 0) 
            throw std::runtime_error ("target mast be a non-negative integer");

        _classificationY[idx] = v;
        classes.insert(v);
    }
    
    std::vector<int> vec;
    vec.insert(vec.begin(), classes.begin(), classes.end());
    std::sort(vec.begin(), vec.end());
    if (vec[0] != 0 || vec[vec.size()-1] != vec.size()-1)
        throw std::runtime_error ("targets must follow sequential integers, starting with 0");
    
    _nClasses = vec.size();
}

void Data::loadRegressionY(std::string filename) {
    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        std::cerr << "could not open file " << filename << std::endl;
        exit(1);
    }
    MM_typecode typecode;

    int rc = mm_read_banner(file, &typecode);
    if (rc != 0) {
        std::string err;
        switch (rc) {
            case MM_PREMATURE_EOF:
                err = "Premature EOF at header";
                break;
            case MM_NO_HEADER:
                err = "Missing Header";
                break;
            case MM_UNSUPPORTED_TYPE:
                err = "Unrecognized Header";
                break;
            default:
                err = "Unexpected return value from mm_read_banner";
        }
        throw std::runtime_error (err);
    }

    if (!mm_is_array(typecode) || !mm_is_real(typecode) || !mm_is_general(typecode))
        throw std::runtime_error ("Unsupported Matrix Market file. Currently, only array real general files are supported for regression targets");

    int M, N;
    rc = mm_read_mtx_array_size(file, &M, &N);
    if (rc != 0) {
        std::string err;
        switch (rc) {
            case MM_PREMATURE_EOF:
                err = "Premature EOF at size info";
                break;
            default:
                err = "Unexpected return value from mm_read_mtx_crd_size";
        }
        throw std::runtime_error (err);
    } else if (M != (int) _nUsers || N != 1) {
        throw std::runtime_error ("Target size does not match features. It must be rows X 1");
    }

    _regressionY.resize(_nUsers);
    
    for (size_t idx = 0; idx < _nUsers; idx++) {
        double v;
        rc = fscanf(file, "%lf", &v);
        if (rc < 1)
            throw std::runtime_error ("Unexpected error while reading the input file");

        _regressionY[idx] = v;
    }
}

// creates a subset that includes the entire data set
DataSubset Data::createSubset() {
    return DataSubset(this);
}
