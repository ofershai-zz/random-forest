#include "RandomForest.h"
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/random.hpp>
#include <ctime>

void RandomForest::trainTrees(int seed) {
    int startTime = time(NULL);
    int tree;
    boost::mt19937 rnd(seed);
    DataSubset ds = _data->createSubset();
    boost::uniform_int<> bagging(0, ds.nUsers() - 1);    
    
    while (true) {
        {
            boost::interprocess::scoped_lock<boost::mutex> lock(_mtx);
            tree = _forest.size();
            if (tree >= _nTrees)
                return;

            _forest.push_back(getTree());

            if (tree * 20 / _nTrees < (tree + 1) * 20 / _nTrees) {
                int diff = time(NULL) - startTime;
                std::cout << "[" << (diff / 60) << std::setw(2) << std::setfill('0') << (diff % 60) << "] " <<
                        tree << " / " << _nTrees << std::endl;
            }

        }
        std::vector<int> indeces(ds.nUsers());
        for (size_t i = 0; i < indeces.size(); i++) {            
            indeces[i] = bagging(rnd);
        }

        DataSubset subset = ds.createSubsetUsers(indeces);     
        
        _forest[tree]->train(subset, _nFeatures);
    }
}

void RandomForest::train() {
    boost::thread_group pool;
    
    std::cout << "Training Random Forest" << std::endl;

    for (int i = 0; i < _nThreads; i++) {
        pool.create_thread(boost::bind(&RandomForest::trainTrees, this, std::time(0) + i));
    }

    pool.join_all();
    assert(_nTrees == _forest.size());
}

void RandomForest::save(std::ofstream &out) {
    out << _forest.size() << " " << _nFeatures << " " << _nClasses << std::endl;
    for (size_t i = 0; i < _forest.size(); i++)
        _forest[i]->save(out);
}

void RandomForest::load(std::ifstream &in) {
    in >> _nTrees >> _nFeatures >> _nClasses;
    _forest.resize(_nTrees);
    for (size_t i = 0; i < _forest.size(); i++) {
        _forest[i] = getTree();
        _forest[i]->load(in);
    }
}
