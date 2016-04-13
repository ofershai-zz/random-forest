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

