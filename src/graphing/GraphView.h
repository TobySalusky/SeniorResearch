//
// Created by Tobiathan on 10/19/21.
//

#ifndef SENIORRESEARCH_GRAPHVIEW_H
#define SENIORRESEARCH_GRAPHVIEW_H


#include "../vendor/glm/ext/matrix_float4x4.hpp"
#include "../util/Util.h"
#include "../gl/Input.h"
#include "../gl/Mesh2D.h"

struct GraphViewUpdateInfo {
    Input& input;
    Rectangle guiRect;
};

struct GraphViewRenderInfo {
    Mesh2D& plot;
    Input& input;
};

struct GraphView {
    Vec2 centerAt = {0.0f, 0.0f};
    float scale = 1.0f;

    [[nodiscard]] glm::mat4 GenProjection() const;

    void Update(GraphViewUpdateInfo info);
    void Render(GraphViewRenderInfo info);

    Vec2 MousePosToCoords(Vec2 mousePos);
    Vec2 MousePosNPToCoords(Vec2 mousePosNP);

    void ReCenter();
private:
    Rectangle guiRect;
    bool enablePanOrDrag = false;
    Vec2 panStart;

    [[nodiscard]] Vec2 GenScaleVector() const;
    [[nodiscard]] Vec2 GenScaleVectorMat() const;
    [[nodiscard]] Vec2 GenAspectRatio() const;
};


#endif //SENIORRESEARCH_GRAPHVIEW_H
