/* 
 * File:   DataSubset.h
 * Author: ofer
 *
 * Created on June 17, 2013, 5:26 PM
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

