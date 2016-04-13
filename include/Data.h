/* 
 * File:   Data.h
 * Author: ofer
 *
 * Created on June 4, 2013, 1:25 PM
 */

#ifndef DATA_H
#define	DATA_H

#include <vector>
#include <string>

class DataSubsetIterator;

class Data {
    friend class DataSubsetIterator;
protected:
    size_t _nUsers;
    size_t _nFeatures;
    size_t _nClasses;

    std::vector<double> _regressionY;
    std::vector<int> _classificationY;
    
    std::vector<int> _featureList;

    virtual void iterator(DataSubsetIterator *iter, int feature, std::vector<int> &indeces) = 0;
    
public:
    virtual ~Data() {}
    
    Data() {
        _nUsers = 0;
        _nFeatures = 0;
        _nClasses = 0;
    }
    virtual void loadFeatures(std::string filename) = 0;
    virtual bool at(int user, int feature, double &v) = 0;
    void filterFeatures(std::string filename);
    void loadClassificationY(std::string filename);
    void loadRegressionY(std::string filename);

    int nClasses() {
        return _nClasses;
    }

    int nFeatures() {
        return _nFeatures;
    }

    int nFilteredFeatures() {
        return _featureList.size();
    }

    int nUsers() {
        return _nUsers;
    }

    int classificationY(int u) {
        return _classificationY[u];
    }

    double regressionY(int u) {
        return _regressionY[u];
    }

    // creates a subset that includes the entire data set
    class DataSubset createSubset();

    friend class DataSubset;
};


#endif	/* DATA_H */

