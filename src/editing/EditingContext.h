//
// Created by Tobiathan on 11/12/21.
//

#ifndef SENIORRESEARCH_EDITINGCONTEXT_H
#define SENIORRESEARCH_EDITINGCONTEXT_H


#include "../Enums.h"
#include "../vendor/glm/vec2.hpp"

#include <vector>

class EditingContext {
public:

    std::vector<glm::vec2> transformStoreInitPoints;

    struct TransformStartInfo {
        glm::vec2 initMousePos;
    };

    void StartTransform(Enums::TransformationType transformType, TransformStartInfo initInfo, const std::vector<glm::vec2>& points) {
        transformationType = transformType;
        transformationActive = true;
        transformStartInfo = initInfo;
        lastMousePos = initInfo.initMousePos;

        if (transformType == Enums::TRANSFORM_SMEAR) {
            transformStoreInitPoints = points; // copy points
        }
    }

    [[nodiscard]] bool IsTransformationActive() const { return transformationActive; }

    [[nodiscard]] Enums::TransformationType GetTransformationType() const { return transformationType; }

    [[nodiscard]] glm::vec2 GetTransformStartPos() const { return transformStartInfo.initMousePos; }


    void CancelTransform() {
        EndTransform();
    }

    void FinalizeTransform() {
        EndTransform();
    }

    void SetLastMousePos(glm::vec2 pos) {
        lastMousePos = pos;
    }

    [[nodiscard]] glm::vec2 GetLastMousePos() const {
        return lastMousePos;
    }

    [[nodiscard]] bool IsDrawingEnabledForClick() const { return !drawingDisabledForClick; }

    void DisableDrawingForClick() {
        drawingDisabledForClick = true;
    }

    void OnMouseUp() {
        drawingDisabledForClick = false;
    }

private:
    bool drawingDisabledForClick = false;
    bool transformationActive;
    Enums::TransformationType transformationType;
    TransformStartInfo transformStartInfo;
    glm::vec2 lastMousePos;

    void EndTransform() {
        transformationActive = false;
        transformStoreInitPoints.clear();
    }
};


#endif //SENIORRESEARCH_EDITINGCONTEXT_H
