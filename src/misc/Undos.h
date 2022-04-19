//
// Created by Tobiathan on 11/23/21.
//

#ifndef SENIORRESEARCH_UNDOS_H
#define SENIORRESEARCH_UNDOS_H

#include <vector>
#include "Undo.h"
#include "../util/Util.h"

class Undos {
public:
    static void Add(Undo* undo) {
	    LOG("Adding undo...\n");
        undos.push_back(std::unique_ptr<Undo>(undo));
    }

    static void UseLast() {
        if (undos.empty()) return;
	    LOG("Using undo...\n");
        undos.back()->Apply();
        undos.pop_back();
    }

    static void PopLast() {
        undos.pop_back();
    }
private:
    static std::vector<std::unique_ptr<Undo>> undos;
};

#endif //SENIORRESEARCH_UNDOS_H
