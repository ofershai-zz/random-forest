#include "mmio.h"
#include "SparseData.h"
#include "DataSubset.h"
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <set>
#include <algorithm>

void SparseData::loadFeatures(std::string filename) {
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

    if (!mm_is_coordinate(typecode) || !mm_is_real(typecode) || !mm_is_general(typecode))
        throw std::runtime_error ("Unsupported Matrix Market file. Currently, only cooridnate real general files are supported for sparse features");

    int M, N, nz;
      
    rc = mm_read_mtx_crd_size(file, &M, &N, &nz);
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

    _nUsers = M; // users must start at 1
    _nFeatures = N + 1; // to account for users either starting at 1 or 0x
    _featureList.resize(N);
    for (int i = 0; i < N; i++) {
        _featureList[i] = i;
    }

    std::cout << "users:" << _nUsers << std::endl;
    std::cout << "features:" << _nFeatures << std::endl;
    std::cout << "nnz:" << nz << std::endl;
    _featureList.resize(_nFeatures);
    for (int idx = 0; idx < _nFeatures; ++idx) {
        _featureList[idx]=idx;
    }
    
    _user.resize(_nUsers); 
    _feature.resize(nz);
    _val.resize(nz);

    int prevX = 0;
    int prevY = -1;

    int x, y;
    double v;

    for (int idx = 0; idx < nz; idx++) {
        rc = fscanf(file, "%d %d %lf", &x, &y, &v);
        x--; // users are indexed at 1 in the matrix market file

        if (rc < 3)
            throw std::runtime_error ("Unexpected error while reading the input file");

        if (x < prevX) {
            std::cerr << "Rows must be monotonically non-decreasing. found " << x << " after " << prevX << std::endl;
            throw std::runtime_error ("Rows must be monotonically non-decreasing");
        } else if (x > prevX) {
            prevY = -1;
            for (; prevX < x; prevX++)
                _user[prevX] = idx;
        }
        if (y <= prevY) {
	        std::cerr << "Columns must be monotonically non-decreasing within a row. found " << y << " after " << prevY << " for user (row) " << x << std::endl;
	        throw std::runtime_error ("Columns must be monotonically non-decreasing within a row");
	    }

        prevY = y;
        _feature[idx] = y;
        _val[idx] = v;

    }
    _user[x] = nz;
  
    transpose();
}

bool SparseData::at(int u, int f, double &v) {
    int start = u == 0 ? 0 : _user[u-1];
    int end = _user[u];
    int idx = (end+start)/2;
    while (end > start) {
        idx = (start + end)/2;
        if (_feature[idx] == f) {
            v = _val[idx];
            return true;
        } else if (_feature[idx] < f) {
            start = idx+1;
        } else {
            end = idx;
        }
    }
    return false;    
}

void SparseData::transpose() {
    _userT.clear();
    _userT.resize(_val.size());
    _featureT.clear();
    _featureT.resize(_nFeatures);
    _valT.clear();
    _valT.resize(_val.size());

    std::vector<int> counts(_nFeatures);
    for (size_t j = 0; j < _feature.size(); j++) {
        counts[_feature[j]]++;
    }

    _featureT[0] = counts[0];
    counts[0] = 0;
    for (size_t j = 1; j < _nFeatures; j++) {
        _featureT[j] = _featureT[j - 1] + counts[j];
        counts[j] = 0;
    }

    int j = 0;
    for (size_t i = 0; i < _nUsers; i++) {
        for (; j < _user[i]; j++) {
            int idx = (_feature[j] <= 0 ? 0 : _featureT[_feature[j] - 1]) + counts[_feature[j]];
            _userT[idx] = i;
            _valT[idx] = _val[j];
            counts[_feature[j]]++;
        }
    }
}

void SparseData::iterator(DataSubsetIterator *iter, int feature, std::vector<int> &indeces) {
    iter->_rows = &_userT;
    iter->_vals = &_valT;
    iter->_idx = 0;
    iter->_size = 0;
    
    int start = feature <= 0 ? 0 : _featureT[feature-1];
    int end = _featureT[feature];
    
    size_t i = 0;
    int idx = start;
    while (i < indeces.size() && idx < end) {
        while (idx < end && indeces[i] > iter->_rows->at(idx))
            idx++;
        if (idx < end) {
            while (i < indeces.size() && indeces[i] < iter->_rows->at(idx))
                i++;
            while (i < indeces.size() && indeces[i] == iter->_rows->at(idx)) {
                iter->_indeces.push_back(idx);
                iter->_size++;
                i++;
            }
        }
    }    
}
