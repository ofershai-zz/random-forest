#include "mmio.h"
#include "DenseData.h"
#include "DataSubset.h"
#include "Data.h"
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <set>
#include <algorithm>

void DenseData::loadFeatures(std::string filename) {    
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
        throw std::runtime_error ("Unsupported Matrix Market file. Currently, only array real general files are supported for dense features");

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
    }
    
    _nFeatures = N;
    _nUsers = M;
    _featureList.resize(_nFeatures);
    for (int idx = 0; idx < _nFeatures; ++idx) {
        _featureList[idx]=idx;
    }

    _features.resize(_nFeatures);
    
    for (size_t f = 0; f < _nFeatures; f++) {
        _features[f].resize(_nUsers);
        for (size_t u = 0; u < _nUsers; u++) {
            double v;
            rc = fscanf(file, "%lf", &v);
            if (rc < 1)
                throw std::runtime_error ("Unexpected error while reading the input file");
                    
            _features[f][u] = v;
        }
    }
}

bool DenseData::at(int u, int f, double &v) {
    v = _features[f][u];
    return true;
}

void DenseData::iterator(DataSubsetIterator *iter, int feature, std::vector<int> &indeces) {
    // dense data set, so use all the indeces.
    iter->_rows = NULL;
    iter->_vals = & (_features[feature]);
    std::vector<int> tmp(indeces);
    iter->_indeces.swap(tmp);
    iter->_idx = 0;
    iter->_size = indeces.size();
}
