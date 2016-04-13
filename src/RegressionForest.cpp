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


#include "RegressionForest.h"
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/random.hpp>
#include <ctime>

double RegressionForest::evaluate(int uid) {
    double retVal = 0;
    for (size_t i = 0; i < _forest.size(); i++) {
        double v;
        ((RegressionTree *) _forest[i])->evaluate(*_data, uid, &v);
        retVal += v;
    }

    return retVal/_forest.size();
}

double RegressionForest::evaluate(int uid, std::vector<int> &permutation, int feature) {
    double retVal = 0;
    int cnt = 0;
    for (size_t i = 0; i < _forest.size(); i++) {
        double v;
        if (((RegressionTree *) _forest[i])->evaluateOOB(*_data, uid, permutation, feature, &v)) {
            retVal += v;
            cnt++;
        }
    }

    return retVal /cnt;
 }

void RegressionForest::evaluateThread(std::vector<double> *vals) {
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
	    vals->at(uid) = evaluate(uid);
    }
}

void RegressionForest::evaluateOOBThread(std::vector<double> *out, std::vector<int> *permutation, int feature) {
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
	    out->at(uid) = evaluate(uid, *permutation, feature);
    }    
}

void RegressionForest::evaluate(std::vector<double> &Y) {
    boost::thread_group pool;
    _counter = 0;
    _increment = 100;
        
    for (int i = 0; i < _nThreads; i++) {
        pool.create_thread(boost::bind(&RegressionForest::evaluateThread, this, &Y));        
    }
    pool.join_all();
}

void RegressionForest::evaluateOOB(std::vector<double> &Y, std::vector<int> &permutation, int feature) {
    boost::thread_group pool;
    _counter = 0;
    _increment = 100;
    
    for (int i = 0; i < _nThreads; i++) {
        pool.create_thread(boost::bind(&RegressionForest::evaluateOOBThread, this, &Y, &permutation, feature));        
    }
    pool.join_all();
}


