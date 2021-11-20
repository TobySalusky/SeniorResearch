//
// Created by Tobiathan on 11/7/21.
//

#ifndef SENIORRESEARCH_LATHE_H
#define SENIORRESEARCH_LATHE_H


#include "ModelObject.h"

class Lathe : public ModelObject {
public:
    void HyperParameterUI(const UIInfo& info) final;
    void AuxParameterUI(const UIInfo& info) final;
    void UpdateMesh() final;
    void ModeSetUI(Enums::DrawMode& drawMode) final;

    void ClearAll() override;


    void RenderSelf2D(RenderInfo2D renderInfo) final;
    void RenderGizmos2D(RenderInfo2D renderInfo) final;

    std::vector<glm::vec2>& GetPointsRefByMode(Enums::DrawMode drawMode) final;

    Enums::LineType LineTypeByMode(Enums::DrawMode drawMode) final;

    std::tuple<std::vector<glm::vec3>, std::vector<GLuint>> GenMeshTuple() final;

private:
    float scaleRadius = 1.0f, scaleZ = 1.0f, scaleY = 1.0f, leanScalar = 0.25f;
    int countPerRing = 10;

    bool wrapStart = false, wrapEnd = false;

    std::vector<glm::vec2> plottedPoints;
    std::vector<glm::vec2> graphedPointsY;
    std::vector<glm::vec2> graphedPointsZ;

    glm::vec4 plotColor = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec4 graphColorY = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 graphColorZ = {0.0f, 1.0f, 0.0f, 1.0f};
};


#endif //SENIORRESEARCH_LATHE_H
