//
// Created by Tobiathan on 11/28/21.
//

#ifndef SENIORRESEARCH_BLENDMODES_H
#define SENIORRESEARCH_BLENDMODES_H


#include "BlendMode.h"
#include "BlendModeManager.h"

class BlendModes {
public:
    static BlendMode* Get(int ID) { return manager.Get(ID); }
    static void Add(BlendMode* newBlendMode) { manager.Add(newBlendMode); }
private:
    static BlendModeManager manager;
};


#endif //SENIORRESEARCH_BLENDMODES_H
