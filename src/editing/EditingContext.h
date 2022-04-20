//
// Created by Tobiathan on 11/12/21.
//

#ifndef SENIORRESEARCH_EDITINGCONTEXT_H
#define SENIORRESEARCH_EDITINGCONTEXT_H


#include "../util/Includes.h"
#include "../gl/Mesh2D.h"
#include "../misc/Undos.h"
#include "../graphing/GraphView.h"

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

        transformStoreInitPoints = points;

        axisLock = Enums::LOCK_NONE;

        localTransform = false;
    }

    [[nodiscard]] bool IsTransformationActive() const { return transformationActive; }

    [[nodiscard]] Enums::TransformationType GetTransformationType() const { return transformationType; }

    [[nodiscard]] glm::vec2 GetTransformStartPos() const { return transformStartInfo.initMousePos; }


    void CancelTransform(std::vector<glm::vec2>& points) {
        Undos::PopLast();
        points.clear();
        points.insert(points.end(), transformStoreInitPoints.begin(),  transformStoreInitPoints.end());
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

    void LockAxis(Enums::TransformAxisLock lockType) { axisLock = lockType; }

    void SetPrimaryGizmoPoint(const Vec2 &primaryGizmoPoint) { EditingContext::primaryGizmoPoint = primaryGizmoPoint; }
    void SetSecondaryGizmoPoint(const Vec2 &secondaryGizmoPoint) { EditingContext::secondaryGizmoPoint = secondaryGizmoPoint; }
//    void SetTertiaryGizmoPoint(const Vec2 &tertiaryGizmoPoint) { EditingContext::tertiaryGizmoPoint = tertiaryGizmoPoint; }

    [[nodiscard]] Enums::TransformAxisLock GetAxisLock() const {
        return axisLock;
    }

    Vec2 GenLockMult() const {
        switch (GetAxisLock()) {
            case Enums::LOCK_NONE:
                return {1.0f, 1.0f};
            case Enums::LOCK_X:
                return {1.0f, 0.0f};
            case Enums::LOCK_Y:
                return {0.0f, 1.0f};
        }
    }

    void RenderGizmos(Mesh2D& plot, GraphView& graphView) {
        const RGBA orange = {1.0f, 0.7f, 0.0f, 0.6f};
        const RGBA blue = {0.2f, 0.1f, 1.0f, 0.6f};
        const RGBA red = {1.0f, 0.1f, 0.3f, 0.6f};

        switch (snapGrid) { // render snapping grids
            case SnapGrid::Circle: {
                constexpr RGBA circleGridColour {0.0f, 0.0f, 0.0f, 0.1f};
	            constexpr RGBA circleGridColourActive {1.0f, 0.0f, 0.0f, 0.1f};

            	const auto CircRad = [&](float radius, const RGBA& colour) {
		            plot.AddPolygonOutline(snapGridCentralPoint, radius, 60, colour, 0.005f);
	            };

            	plot.AddPolygonOutline(snapGridCentralPoint,
									0.01f, 20, Util::WithAlpha(circleGridColourActive, 0.3f), 0.01f);

            	for (size_t i = 0; i < 10; i++) { // TODO: calc num of circles to properly fill screen (min thru max)
		            CircRad(0.125f * (float) (i + 1), circleGridColour);
            	}

            	if (isDrawing) {
		            CircRad(graphView.scale * glm::length(drawingBeginCoords - snapGridCentralPoint), circleGridColourActive);
            	}
            }
	        default:
	            break;
        }

        if (transformationActive) {
            switch (transformationType) {
                case Enums::TRANSFORM_DRAG: {
                    plot.AddLines({primaryGizmoPoint, secondaryGizmoPoint}, orange, 0.005f);
                    break;
                }
                case Enums::TRANSFORM_ROTATE: {
                    plot.AddLines({primaryGizmoPoint, secondaryGizmoPoint}, orange, 0.005f);

                    Vec2List rotateGizmo;
                    float initAngle = Util::Angle(GetTransformStartPos() - primaryGizmoPoint);
                    float newAngle = Util::Angle(secondaryGizmoPoint - primaryGizmoPoint);
                    float angleDiff = newAngle - initAngle;

                    const float angleStep = 0.1f, gizmoRad = 0.2f * plot.GenScaleLineMult();

                    int pCount = (int) (std::fabs(angleDiff) / angleStep);
                    rotateGizmo.reserve(pCount + 2);
                    rotateGizmo.emplace_back(primaryGizmoPoint);
                    rotateGizmo.emplace_back(primaryGizmoPoint + glm::normalize(GetTransformStartPos() - primaryGizmoPoint) * gizmoRad);
                    for (int i = 0; i < pCount; i++) {
                        rotateGizmo.emplace_back(primaryGizmoPoint + Util::Polar(gizmoRad, (-initAngle + (float) M_PI_2) + (float) i * angleStep * (float) -Util::Signum(angleDiff)));
                    }

                    plot.AddLines(rotateGizmo, blue, 0.005f);
                    break;
                }
                case Enums::TRANSFORM_SCALE: {
                    plot.AddLines({primaryGizmoPoint, secondaryGizmoPoint}, orange, 0.005f);
                    const Vec2 dir = glm::normalize(secondaryGizmoPoint - primaryGizmoPoint);
                    const auto PointAlongAt = [&](Vec2 magTowards) {
                        return primaryGizmoPoint + (glm::length(magTowards - primaryGizmoPoint) * dir);
                    };
                    const RGBA diffColor = glm::length(GetTransformStartPos() - primaryGizmoPoint) <
                                           glm::length(secondaryGizmoPoint - primaryGizmoPoint) ? blue : red;
                    plot.AddLines({PointAlongAt(GetTransformStartPos()), PointAlongAt(secondaryGizmoPoint)}, diffColor,
                                  0.005f);
                    break;
                }
                case Enums::TRANSFORM_SMEAR: {
                    break;
                }
            }
        }
    }

    [[nodiscard]] bool IsLocalTransform() const {
        return localTransform;
    }

    void MakeLocal() { localTransform = true; }

    [[nodiscard]] bool IsDrawing() const { return isDrawing; }

    void BeginDrawing(Vec2 _drawingBeginCoords) {
    	isDrawing = true;
	    drawingBeginCoords = _drawingBeginCoords;
    }

    void EndDrawing() { isDrawing = false; }

    [[nodiscard]] bool CanUndo() const {
        return !IsDrawing() && !IsTransformationActive();
    }

    [[nodiscard]] bool CanUndoHold() const { return undoTimer <= 0.0f; }

    void UseUndo(bool initUndo) {
        undoTimer = initUndo ? undoTimerInitMax : undoTimerRegMax;
    }

    void Update(float deltaTime) {
        undoTimer -= deltaTime;
    }

    void SetSnapGrid(SnapGrid _snapGrid) { snapGrid = _snapGrid; }

    Vec2 ApplySnapGridIfAny(Vec2 actualCanvasPos);

private:
    static constexpr float undoTimerInitMax = 0.3f, undoTimerRegMax = 0.09f;
    float undoTimer = 0.0f;
    bool isDrawing;
    Vec2 drawingBeginCoords;

    bool localTransform;
    Enums::TransformAxisLock axisLock;
    bool drawingDisabledForClick = false;
    bool transformationActive = false;
    Enums::TransformationType transformationType;
    TransformStartInfo transformStartInfo;
    Vec2 lastMousePos;

    SnapGrid snapGrid = SnapGrid::None;
    Vec2 snapGridCentralPoint = {0.0f, 0.0f};

    Vec2 primaryGizmoPoint, secondaryGizmoPoint/*, tertiaryGizmoPoint*/;

    void EndTransform() {
        transformationActive = false;
        transformStoreInitPoints.clear();
    }
};


#endif //SENIORRESEARCH_EDITINGCONTEXT_H
