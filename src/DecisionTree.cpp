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


#include "DecisionTree.h"
#include <algorithm>
#include <iostream>
#include <cassert>

DecisionTree::DecisionTree() : Tree() {
}

bool DecisionTree::isSingleClass(DataSubset &data, const std::vector<int> &users) {
    int cls = data.classificationY(users[0]);
    for (size_t i = 1; i < users.size(); i++)
        if (data.classificationY(users[i]) != cls)
            return false;

    return true;
}

int DecisionTree::mostPopularClass(DataSubset &data, const std::vector<int> &users) {
    std::vector<int> classes(data.nClasses());
    for (size_t i = 0; i < users.size(); i++)
        classes[data.classificationY(users[i])] += 1;

    int mx = classes[0];
    int idx = 0;
    for (size_t i = 1; i < classes.size(); i++) {
        if (classes[i] > mx) {
            mx = classes[i];
            idx = i;
        }            
    }
    return idx;
}

bool DecisionTree::checkAndMakeLeaf(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int depth, bool forceLeaf) {
    node->_isLeaf = false;
    if (userIndeces.size() == 0) {
        node->_isLeaf = true;
        node->_cls = -1;
    } else if (isSingleClass(data, userIndeces)) {
        node->_isLeaf = true;
        node->_cls = data.classificationY(userIndeces[0]);
    } else if (forceLeaf || (_maxDepth > 0 && depth >= _maxDepth) || userIndeces.size() < _minSplit) {
        node->_isLeaf = true;
        node->_cls = mostPopularClass(data, userIndeces);        
    }
    return node->_isLeaf;
}

void DecisionTree::bestThreshold(DataSubset &data, int feature, double &bestGini, double &bestSplit) {
    DataSubsetIterator users(data, feature);

    std::vector<double> noValueCounts(data.nClasses());
    std::vector<double> moreThan(data.nClasses());
    std::vector<double> lessThan(data.nClasses());
    std::vector<std::pair<double, int> > values;
    values.reserve(users.size());

    // figure out the counts for the no-values split
    // figure out the counts for each class

    const std::vector<int> &userIndeces = data.getUsers();
    size_t u = 0;
    while (u < userIndeces.size()) {
        assert(!users.hasNext() || users.index() >= userIndeces[u]);
        while (u < userIndeces.size() && (!users.hasNext() || users.index() > userIndeces[u])) {
            noValueCounts[data.classificationY(userIndeces[u])] += 1;
            u++;
        }
        while (users.hasNext() && u < userIndeces.size() && users.index() == userIndeces[u]) {
            int t = data.classificationY(users.index());
            values.push_back(std::pair<double, int>(users.value(), t));
            moreThan[t] += 1;
            users.next();
            u++;
        }
    }

    // compute the gini index for the no-value set
    double gNoValue = 0;
    double Nnv = 0;
    double N = data.nUsers();
    for (size_t i = 0; i < noValueCounts.size(); ++i) {
        Nnv += noValueCounts[i];
        gNoValue += noValueCounts[i] * noValueCounts[i];
    }
    // gNoValue = (1 - gNoValue / Nnv / Nnv) * Nnv / N;
    if (Nnv > 0)
        gNoValue = (Nnv - gNoValue / Nnv) / N;

    if (values.size() == 0) {
        bestGini = gNoValue;
        bestSplit = 0;
        return;
    }
    
    // sort the values
    // we are starting with "everything is bigger than alpha"
    std::sort(values.begin(), values.end());
    int nMore = users.size();
    int nLess = 0;
    double split = values[0].first - 1;
    double maxVal = values[values.size() - 1].first;
    size_t index = 0;

    double gini = 0;
    for (size_t i = 0; i < moreThan.size(); i++) {
        gini += moreThan[i] * moreThan[i];
    }
    // gini = (1 - gini / nMore / nMore) * nMore / N;
    gini = (nMore - gini / nMore) / N;
    
    bestGini = gini;
    bestSplit = split;

    assert(gini + gNoValue <= 1 && gini + gNoValue >= 0);

    // walk the sorted values, and compute the gini value. keep track of the minimum
    while (split < maxVal) {
        // find the next split value
        split = values[index].first;
        while (index < values.size() && values[index].first <= split) {
            moreThan[values[index].second]--;
            nMore--;
            lessThan[values[index].second]++;
            nLess++;
            index++;
        }
        if (index < values.size()) {
            split = index >= values.size() ? values[index - 1].first + 1 : (values[index - 1].first + values[index].first) / 2;

            double giniLess = 0;
            double giniMore = 0;

            for (size_t i = 0; i < moreThan.size(); i++) {
                giniLess += lessThan[i] * lessThan[i];
                giniMore += moreThan[i] * moreThan[i];
            }
            giniLess = (nLess - giniLess / nLess) / N;
            giniMore = (nMore - giniMore / nMore) / N;

            gini = giniLess + giniMore;

            assert(gini + gNoValue <= 1 && gini + gNoValue >= 0);

            if (gini < bestGini) {
                bestGini = gini;
                bestSplit = split;
            }
        }        
    }

    bestGini += gNoValue;
}

void DecisionTree::evaluate(Data &data, int uid, int *out) {
    Node *node = getNode(data, uid);
 
    *out = node->_cls;
    
    }


bool DecisionTree::evaluateOOB (Data &data, int uid, std::vector<int> &permutation, int feature, int *out) {
    Node *node = getNodeOOB(data, uid, permutation, feature);
    if (!node) {
        *out = -1;
        return false;
    }
    
    *out = node->_cls;
    return true;
}

void DecisionTree::load(Node *node, std::ifstream &in) {
    node->_noValue = NULL;
    node->_greaterThan = NULL;
    node->_lessThan = NULL;

    in >> node->_isLeaf >>
            node->_cls >>
            node->_threshold >>
            node->_feature; 
        
    if (!node->_isLeaf) {
        node->_noValue = new Node;
        load(node->_noValue, in);        
        node->_greaterThan = new Node;
        load(node->_greaterThan, in);        
        node->_lessThan = new Node;
        load(node->_lessThan, in);        
    }
}

void DecisionTree::save(Node *node, std::ofstream &out) {
    assert(!node->_isLeaf || (node->_noValue == NULL && node->_greaterThan == NULL && node->_lessThan == NULL));
    assert(node->_isLeaf || (node->_noValue != NULL && node->_greaterThan != NULL && node->_lessThan != NULL));
    
    out << node->_isLeaf << " " << node->_cls << " " << node->_threshold << " " << node->_feature << std::endl;    
    if (!node->_isLeaf) {
        save(node->_noValue, out);
        save(node->_greaterThan, out);
        save(node->_lessThan, out);
    }
}
