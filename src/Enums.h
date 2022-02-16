//
// Created by Tobiathan on 11/7/21.
//

#ifndef SENIORRESEARCH_ENUMS_H
#define SENIORRESEARCH_ENUMS_H


class Enums {
public:
// Enums

    enum Direction {
        HORIZONTAL, VERTICAL
    };
    enum DrawMode {
        MODE_PLOT, MODE_GRAPH_Y, MODE_GRAPH_Z, MODE_CROSS_SECTION
    };
    enum TransformationType {
        TRANSFORM_DRAG, TRANSFORM_ROTATE, TRANSFORM_SCALE, TRANSFORM_SMEAR
    };
    enum LineType {
        POLYLINE, PIECEWISE
    };
    enum BlendType {
        CUSTOM, LINEAR, SINE, ELASTIC_IN, ELASTIC_OUT, ELASTIC_INOUT
    };
    enum ModelObjectType {
        LATHE, CROSS_SECTIONAL
    };
};


#endif //SENIORRESEARCH_ENUMS_H
