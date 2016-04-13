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


#ifndef DENSEDATA_H
#define	DENSEDATA_H

#include "Data.h"

class DenseData : public Data {
protected:
    std::vector<std::vector<double> > _features; // [feature][user]
public:

    DenseData() : Data() {
    }

    virtual void iterator(DataSubsetIterator *iter, int feature, std::vector<int> &indeces);
    virtual void loadFeatures(std::string filename);
    virtual bool at(int user, int feature, double &v);
};

#endif	/* DENSEDATA_H */

