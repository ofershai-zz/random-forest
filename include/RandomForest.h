/* 
 * File:   RandomForest.h
 * Author: ofer
 *
 * Created on June 25, 2013, 8:12 PM
 */

#ifndef RANDOMFOREST_H
#define	RANDOMFOREST_H

#include "Tree.h"
#include "DecisionTree.h"
#include "RegressionTree.h"
#include <boost/thread.hpp>

class RandomForest {
protected:
    Data *_data;
    int _nTrees;
    int _nFeatures;
    int _nThreads;
    int _nClasses; // only used in ClassificationForest, but doesn't hurt RegressionForest, so leave here to use more code in common

    std::vector<Tree *> _forest;
    
    int _counter;
    int _increment;
    boost::mutex _mtx;
    
    void trainTrees(int seed);

    virtual Tree * getTree()  = 0;
    
public:
    RandomForest(Data *data, int nTrees, int nFeatures, int nThreads) : _data(data), 
        _nTrees(nTrees), _nFeatures(nFeatures), _nThreads(nThreads) {
        _nClasses = data->nClasses();
    }
    
    
    virtual ~RandomForest() {}
    
    void train();
    void save(std::ofstream &out);
    void load(std::ifstream &in);
    int nClasses() { return _nClasses; }

    void findUsedFeatures(std::vector<bool> &features) {
        for (int i = 0; i < _nTrees; i++) {
	    _forest[i]->findUsedFeatures(features);
	}
    }
};

#endif	/* RANDOMFOREST_H */

