#include "Tree.h"
#include "ExecutionConfiguration.h"
#include <algorithm>
#include <iostream>
#include <cassert>

Tree::Tree() : _maxDepth(0), _minSplit(0) {
    if (ExecutionConfiguration::intExists("maxdepth"))
        _maxDepth = ExecutionConfiguration::getInt("maxdepth");
    if (ExecutionConfiguration::intExists("minsplit"))
        _minSplit = ExecutionConfiguration::getInt("minsplit");
}

void Tree::Node::splitData(DataSubset &data, std::vector<int> &noVal, std::vector<int> &greater, std::vector<int> &less) {
    DataSubsetIterator u(data, _feature);
    noVal.reserve(data.nUsers() - u.size());
    greater.reserve(u.size());
    less.reserve(u.size());

    std::vector<int> users = data.getUsers();

    for (size_t i = 0; i < users.size(); i++) {
        if (u.hasNext() && users[i] == u.index()) {
            if (u.value() < _threshold) {
                less.push_back(users[i]);
            }
            else {
                greater.push_back(users[i]);
            }
            u.next();
        } else {
            noVal.push_back(users[i]);
        }
    }
}

void Tree::train(DataSubset &data, int nFeatures) {
    std::vector<int> tmp(data.getUsers());
    _users.swap(tmp);
    train(data, data.getUsers(), &_root, nFeatures, 0);
}

void Tree::save(std::ofstream &out) {
    save(&_root, out);
}
void Tree::load(std::ifstream &in) {
    load(&_root, in);
}

void Tree::Node::findUsedFeatures(std::vector<bool> &features) {
  if (!_isLeaf) {
      features[_feature] = true;
      _noValue->findUsedFeatures(features);
      _greaterThan->findUsedFeatures(features);
      _lessThan->findUsedFeatures(features);
  }
}

void Tree::train(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int nFeatures, int depth) {    
    if (checkAndMakeLeaf(data, userIndeces, node, depth))
        return;

    node->_isLeaf = false;
    node->_val = 0;

    DataSubset dataSubset = data.createSubsetUsers(userIndeces);
    double bestVal = 1e100;
    double bestSplit = 0;
    int bestFeature = -1;
    double val, split;
    if (nFeatures < data.nFeatures()) {
        std::vector<int> features = data.getSomeFeatures(nFeatures);

        for (std::vector<int>::iterator f = features.begin(); f != features.end(); ++f) {
            bestThreshold(dataSubset, *f, val, split);
            if (val < bestVal) {
                bestVal = val;
                bestSplit = split;
                bestFeature = *f;
            }
        }
    } else {
        for (int f = 0; f < dataSubset.nFeatures(); f++) {
            bestThreshold(dataSubset, f, val, split);
            if (val < bestVal) {
                bestVal = val;
                bestSplit = split;
                bestFeature = f;
            }
        }            
    }                    
    node->_feature = bestFeature;
    node->_threshold = bestSplit;
    
    std::vector<int> noValue;
    std::vector<int> greaterThan;
    std::vector<int> lessThan;

    node->splitData(dataSubset, noValue, greaterThan, lessThan);
    
    if (noValue.size() == userIndeces.size() || greaterThan.size() == userIndeces.size() || lessThan.size() == userIndeces.size()) {
        // a rare edge case where there is no possible split of the data based on the features
        checkAndMakeLeaf(data, userIndeces, node, 0, true);
    } else {
        node->_noValue = new Node;
        train(dataSubset, noValue, node->_noValue, nFeatures, depth+1);
        node->_greaterThan = new Node;
        train(dataSubset, greaterThan, node->_greaterThan, nFeatures, depth+1);
        node->_lessThan = new Node;
        train(dataSubset, lessThan, node->_lessThan, nFeatures, depth+1);
    }
}

Tree::Node *Tree::getNode(Data &data, int uid) {
    Node *node = &_root;
 
    while (!node->_isLeaf) {
        double v;
        if (data.at(uid, node->_feature, v)) {
            if (v < node->_threshold)
                node = node->_lessThan;
            else
                node = node->_greaterThan;
        }
        else {
            node = node->_noValue;
        }
    }
    
    return node;
}


Tree::Node *Tree::getNodeOOB (Data &data, int uid, std::vector<int> &permutation, int feature) {
    if (std::binary_search(_users.begin(), _users.end(), uid))
        return NULL;

    Node *node = &_root;
    while (!node->_isLeaf) {
        double v;
        bool valueExists;
        if (node->_feature == feature) {
            assert(uid < permutation.size());
            valueExists = data.at(permutation[uid], feature, v);
        }
        else
            valueExists = data.at(uid, node->_feature, v);
        
        if (valueExists) {
            if (v < node->_threshold)
                node = node->_lessThan;
            else
                node = node->_greaterThan;
        }
        else
            node = node->_noValue;
    }
    return node;
}
