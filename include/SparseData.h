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


#ifndef SPARSEDATA_H
#define	SPARSEDATA_H

#include "Data.h"
#include <vector>
#include <string>

class SparseData : public Data {
protected:

    std::vector<int> _user;
    std::vector<int> _feature;
    std::vector<double> _val;

    std::vector<int> _userT;
    std::vector<int> _featureT;
    std::vector<double> _valT;

    void transpose();

public:

    SparseData() : Data() {
    }

    virtual void iterator(DataSubsetIterator *iter, int feature, std::vector<int> &);
    virtual void loadFeatures(std::string filename);
    virtual bool at(int user, int feature, double &v);
};


#endif	/* DATA_H */

