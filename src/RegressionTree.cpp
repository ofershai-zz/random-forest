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


#include "RegressionTree.h"
#include "DataSubset.h"
#include <algorithm>
#include <iostream>
#include <cassert>

RegressionTree::RegressionTree() : Tree() {
}

bool RegressionTree::checkAndMakeLeaf(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int depth, bool forceLeaf) {
    node->_isLeaf = false;
    if (userIndeces.size() == 0) {
        node->_isLeaf = true;
        node->_val = 0;
    } else if (isSingleValue(data, userIndeces)) {
        node->_isLeaf = true;
        node->_val = data.regressionY(userIndeces[0]);
    } else if (forceLeaf || (_maxDepth > 0 && depth >= _maxDepth) || userIndeces.size() < _minSplit) {
        node->_isLeaf = true;
        node->_val = averageValue(data, userIndeces);        
    } 
    return node->_isLeaf;
}

bool RegressionTree::isSingleValue(DataSubset &data, const std::vector<int> &users) {
    float val = data.regressionY(users[0]);
    for (size_t i = 1; i < users.size(); i++)
        if (data.regressionY(users[i]) != val)
            return false;

    return false;
}

double RegressionTree::averageValue(DataSubset &data, const std::vector<int> &users) {
    double val = 0;
    for (size_t i = 0; i < users.size(); i++) 
        val += data.regressionY(users[i]);

    return val/users.size();
}

void RegressionTree::bestThreshold(DataSubset &data, int feature, double &bestSE, double &bestSplit) {
    DataSubsetIterator users(data, feature);

    double noValueSum = 0, noValueSum2 = 0;
    double moreThanSum = 0, moreThanSum2 = 0;
    double lessThanSum = 0, lessThanSum2 = 0;
    int noValueCnt = 0, moreThanCnt = 0, lessThanCnt = 0;
    
    std::vector<std::pair<double, double> > values;
    values.reserve(users.size());

    // initialize the counts and sums
    const std::vector<int> &userIndeces = data.getUsers();
    size_t u = 0;
    while (u < userIndeces.size()) {
        assert(!users.hasNext() || users.index() >= userIndeces[u]);
        while (u < userIndeces.size() && (!users.hasNext() || users.index() > userIndeces[u])) {
            double y = data.regressionY(userIndeces[u]);
            noValueCnt++;
            noValueSum += y;
            noValueSum2 += y * y;
            u++;
        }
        // everything with a value goes to moreThan at this point
        while (users.hasNext() && u < userIndeces.size() && users.index() == userIndeces[u]) {
            double y = data.regressionY(users.index());
            values.push_back(std::pair<double, double>(users.value(), y));
            moreThanCnt++;
            moreThanSum += y;
            moreThanSum2 += y * y;
            users.next();
            u++;
        }
    }

    // compute the sum of squared errors
    double noValueSE = 0;
    if (noValueCnt > 0)
        noValueSE = noValueSum2 - noValueSum * noValueSum / noValueCnt;

    if (values.size() == 0) {
        bestSE = noValueSE;
        bestSplit = 0;
        return;
    }
    
    // we are starting with "everything is bigger than alpha"
    // sort the values
    std::sort(values.begin(), values.end());
    double split = values[0].first - 1;
    double maxVal = values[values.size() - 1].first;
    size_t index = 0;

    double moreThanSE = moreThanSum2 - moreThanSum * moreThanSum / moreThanCnt;
    double lessThanSE = 0;
    double SE = moreThanSE;

    bestSE = SE;
    bestSplit = split;
    
    assert(SE >= -1e-6);

    // walk the sorted values, and compute the squared error. keep track of the minimum
    while (split < maxVal) {
        // find the next split value
        split = values[index].first;
        while (index < values.size() && values[index].first <= split) {
            double y = values[index].second;            
            moreThanSum -= y;
            moreThanSum2 -= y * y;
            moreThanCnt--;

            lessThanSum += y;
            lessThanSum2 += y * y;
            lessThanCnt++;
            index++;
        }
        if (index < values.size()) {
            split = index >= values.size() ? values[index - 1].first + 1 : (values[index - 1].first + values[index].first) / 2;

            moreThanSE = moreThanSum2 - moreThanSum * moreThanSum / moreThanCnt;
            lessThanSE = lessThanSum2 - lessThanSum * lessThanSum / lessThanCnt;
            SE = moreThanSE + lessThanSE;
            
            assert(moreThanSE >= -1e-6 && lessThanSE >= -1e-6);

            if (SE < bestSE) {
                bestSE = SE;
                bestSplit = split;
            }
        }
    }

    bestSE = -bestSE - noValueSE;
}

void RegressionTree::evaluate(Data &data, int uid, double *out) {
    Node *node = getNode(data, uid);

    *out = node->_val;
}


bool RegressionTree::evaluateOOB (Data &data, int uid, std::vector<int> &permutation, int feature, double *out) {
    Node *node = getNodeOOB(data, uid, permutation, feature);
    if (!node) 
        return false;
    
    *out = node->_val;
    return true;
}

void RegressionTree::load(Node *node, std::ifstream &in) {
    node->_noValue = NULL;
    node->_greaterThan = NULL;
    node->_lessThan = NULL;

    in >> node->_isLeaf >>
            node->_val >>
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

void RegressionTree::save(Node *node, std::ofstream &out) {
    assert(!node->_isLeaf || (node->_noValue == NULL && node->_greaterThan == NULL && node->_lessThan == NULL));
    assert(node->_isLeaf || (node->_noValue != NULL && node->_greaterThan != NULL && node->_lessThan != NULL));
    
    out << node->_isLeaf << " " << node->_val << " " << node->_threshold << " " << node->_feature << std::endl;    
    if (!node->_isLeaf) {
        save(node->_noValue, out);
        save(node->_greaterThan, out);
        save(node->_lessThan, out);
    }
}


// precondition: values is sorted in increasing order
/*
double giniIndex(std::vector<double> &values) {
    std::cout << "gini Index" << std::endl;
    
    int size = values.size();
    double denum = 0;
    double num = 0;
    for (size_t i = 0; i < size; i++) {
        num += (size - i) * values[i];
        denum += values[i];
        std::cout << i << " " << values[i] << " " << num << " " << denum << std::endl;
    }
    if (denum > 0) {
        std::cout << "returning: " << 1 + (1 - 2 * num / denum) / values.size() << std::endl;
        return 1 + (1 - 2 * num / denum) / values.size();
    }
    return 0;
}

void insertSorted(std::vector<double> &values, double newVal) {
    values.push_back(newVal);   
    int pos = values.size()-1;   
    while (pos >= 1 && values[pos] < values[pos-1]) {
        double tmp = values[pos];
        values[pos] = values[pos-1];
        values[pos-1] = tmp;
        pos--;
    }
}

void removeSorted(std::vector<double> &values, double remVal) {
    int pos = 0;
    while (pos < values.size() && values[pos] < remVal)
        pos++;

    assert(pos < values.size() && values[pos] == remVal);`
    
    while (pos < values.size()) {
        values[pos] = values[pos+1];
        pos++;
    }
    
    values.resize(values.size()-1);
    
}

 void RegressionTree::bestThresholdGini(DataSubset &data, int feature, double &bestGini, double &bestSplit) {
    DataSubsetIterator users(data, feature);
    std::vector<double> noValues;
    std::vector<double> greaterThan;
    std::vector<double> lessThan;
    std::vector<std::pair<double, double> > values;
    noValues.reserve(users.size());
    values.reserve(users.size());

    //const std::vector<int> userIndeces;
    const std::vector<int> &userIndeces = data.getUsers();
    size_t u = 0;
    while (u < userIndeces.size()) {
        assert(!users.hasNext() || users.index() >= userIndeces[u]);
        while (u < userIndeces.size() && (!users.hasNext() || users.index() > userIndeces[u])) {
            noValues.push_back(data.regressionY(users.index()));
            u++;
        }
        while (users.hasNext() && u < userIndeces.size() && users.index() == userIndeces[u]) {
            double t = data.regressionY(users.index());
            values.push_back(std::pair<double, double>(users.value(), t));
            // everything starts in greaterThan
            greaterThan.push_back(t);
            users.next();
            u++;
        }
    }
    
    double N = values.size();        

    // compute the gini for the noValues
    std::sort(noValues.begin(), noValues.end());
    double gnv = giniIndex(noValues) * noValues.size() / N;
    size_t index = 0;

    // find the best threshold
    double maxVal = values[values.size()-1].first;
    double split = values[0].first - 1;

    std::sort(values.begin(), values.end());
    std::sort(greaterThan.begin(),greaterThan.end());
    double gini = giniIndex(greaterThan);
    bestGini = gini;
    bestSplit = split;
    
    assert(gini + gnv <= 1 && gini + gnv >= 0);
    
    while (split < maxVal) {
        split = values[index].first;
        while (index < values.size() && values[index].first <= split) {
            removeSorted(greaterThan, values[index].second);
            insertSorted(lessThan, values[index].second);
            index++;
        }
        if (index < values.size()) {
            split = index >= values.size() ? values[index - 1].first + 1 : (values[index - 1].first + values[index].first) / 2;

            double giniLess = giniIndex(greaterThan);
            double giniMore = giniIndex(lessThan);

            gini = (giniLess * lessThan.size() + giniMore * greaterThan.size()) / N;

            assert(gini + gnv <= 1 && gini + gnv >= 0);

            if (gini < bestGini) {
                bestGini = gini;
                bestSplit = split;
            }
        }        
    }

    
    bestGini = bestGini + gnv;    
}

 */
