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


#include "ClassificationForest.h"
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/random.hpp>
#include <ctime>

void ClassificationForest::evaluate(int uid, std::vector<double> &prob) {
    prob.clear();
    prob.resize(_nClasses);
    int cnt = 0;
    for (size_t i = 0; i < _forest.size(); i++) {
        int v;
        ((DecisionTree *)_forest[i])->evaluate(*_data, uid, &v);
        assert(v < _nClasses);

        if (v >= 0) {
            prob[v]++;
            cnt++;
        }
    }

    for (size_t i = 0; i < prob.size(); i++) {
        prob[i] = prob[i] / cnt;
    }
}

void ClassificationForest::evaluate(int uid, std::vector<double> &prob, std::vector<int> &permutation, int feature) {
    prob.clear();
    prob.resize(_nClasses);
    int cnt = 0;
    for (size_t i = 0; i < _forest.size(); i++) {
        int v;
        if (((DecisionTree *)_forest[i])->evaluateOOB(*_data, uid, permutation, feature, &v)) {
            assert(v < _nClasses);
            if (v >= 0)
                prob[v]++;
            cnt++;
        }
    }

    for (size_t i = 0; i < prob.size(); i++) {
        prob[i] = prob[i] / cnt;
    }
}

void ClassificationForest::evaluateThread(std::vector<std::vector<double> > *prob) {
    bool done = false;
    while (!done) {
        int start, end;
        {
            boost::interprocess::scoped_lock<boost::mutex> lock(_mtx);
            start = _counter;
            end = _counter + _increment;
            if (end > _data->nUsers()) {
                end = _data->nUsers();
                done = true;
            }
            _counter = end;
        }
        for (int uid = start; uid < end; uid++)
	    evaluate(uid, prob->at(uid));
    }
}

void ClassificationForest::evaluateOOBThread(std::vector<std::vector<double> > *prob, std::vector<int> *permutation, int feature) {
    bool done = false;
    while (!done) {
        int start, end;
        {
            boost::interprocess::scoped_lock<boost::mutex> lock(_mtx);
            start = _counter;
            end = _counter + _increment;
            if (end > _data->nUsers()) {
                end = _data->nUsers();
                done = true;
            }
            _counter = end;
        }
        for (int uid = start; uid < end; uid++) 
	    evaluate(uid, prob->at(uid), *permutation, feature);
    }    
}

void ClassificationForest::evaluate(std::vector<std::vector<double> > &prob) {  
    boost::thread_group pool;
    _counter = 0;
    _increment = 10;
        
    for (int i = 0; i < _nThreads; i++) {
        pool.create_thread(boost::bind(&ClassificationForest::evaluateThread, this, &prob));        
    }
    pool.join_all();
}

void ClassificationForest::evaluateOOB(std::vector<std::vector<double> > &prob, std::vector<int> &permutation, int feature) {
    boost::thread_group pool;
    _counter = 0;
    _increment = 10;
    
    for (int i = 0; i < _nThreads; i++) {
        pool.create_thread(boost::bind(&ClassificationForest::evaluateOOBThread, this, &prob, &permutation, feature));        
    }
    pool.join_all();
}


