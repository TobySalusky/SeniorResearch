//
// Created by Tobiathan on 11/13/21.
//

#ifndef SENIORRESEARCH_TIMELINE_H
#define SENIORRESEARCH_TIMELINE_H


#include "../gl/Mesh2D.h"
#include "../gl/RenderTarget.h"
#include "../gl/GLWindow.h"
#include "../gl/shaders/Shader2D.h"
#include "../util/Rectangle.h"
#include "../generation/ModelObject.h"
#include "KeyFrameLayer.h"
#include "Animator.h"
#include <vector>
#include <unordered_map>
#include <variant>
#include <functional>

struct TimelineRenderInfo {
    Shader2D& shader2D;
    Enums::DrawMode drawMode;
    const Input& input;
};

template <class T>
struct KeyFrameRowSelection {
    KeyFrameLayer<T>* layer;
    std::vector<KeyFrame<T>*> frames;
    int index;

    bool HasKeyFrameAtTime(float time) {
        for (auto* keyFrame : frames) {
            if (keyFrame->time == time) return true;
            if (keyFrame->time > time) return false;
        }
        return false;
    }
};

struct TimelineSelection {
#define TIMELINE_SELECTION_HANDLE_BOTH(a) {HandleAll(a, a);}
#define TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(a) {HandleAllCapture(a, a);}

    std::vector<std::variant<KeyFrameRowSelection<Vec2List>, KeyFrameRowSelection<float>>> rowSelections;

    bool CrossCompare(TimelineSelection& other) {
        bool returnVal = false;

        const auto CompareFunc = [&](const auto& val) {
            for (int i = 0; i < val.frames.size(); i++) {
                if (other.ContainsKeyframe(val.frames[i])) {
                    returnVal = true;
                    break;
                }
            }
        };

        TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(CompareFunc);

        return returnVal;
    }

    void DeleteAll() {
        const auto DeleteFunc = [](const auto& val) {
            for (int i = val.frames.size() - 1; i >= 0; i--) {
                val.layer->RemoveAtTime((val.frames[i]->time));
            }
        };

        TIMELINE_SELECTION_HANDLE_BOTH(DeleteFunc);
    }

    void MoveAll(float amount) {
        const auto MoveFunc = [=](const auto& val) {
            for (int i = 0; i < val.frames.size(); i++) {
                val.layer->MoveFromTimeToTime(val.frames[i]->time, (val.frames[i]->time) + amount);
            }
        };

        TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(MoveFunc);
    }

    int CountAll() {
        int count = 0;
        const auto CountFunc = [&](const auto& val) {
            count += val.frames.size();
        };
        TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(CountFunc);
        return count;
    }

    bool MoveAllRounded(float amount) {

        bool failed = false;
        const auto CheckFunc = [&](const auto& val) {
            if (failed) return;

            for (auto* keyFramePtr : val.frames) {
                const float time = keyFramePtr->time;
                const float newTime = std::round((time + amount) * 10.0f) / 10.0f;

                if (val.layer->HasKeyFrameAtTime(newTime) && !val.HasKeyFrameAtTime(newTime)) {
                    failed = true;
                    return;
                }
            }
        };

        if (failed) return false;

        const auto MoveFunc = [=](const auto& val) {
            for (auto* keyFramePtr : val.frames) {
                const float time = keyFramePtr->time;
                const float newTime = std::round((time + amount) * 10.0f) / 10.0f;
                val.layer->MoveFromTimeToTime(time, newTime);
            }
        };

        TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(MoveFunc);

        return true;
    }

    void SetBlendID(int num) {
        if (num >= BlendModes::GetNextID()) {
            printf("Error: can not set to blend mode that doesn't exist!\n");
            return;
        }

        const auto SetFunc = [=](const auto& val) {
            for (auto* keyFramePtr : val.frames) {
                keyFramePtr->blendModeID = num;
            }
        };

        TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(SetFunc);
    }

    int CountKeyframes() {
        int count = 0;
        const auto CountFunc = [&](const auto& val) {
            for (auto* keyFramePtr : val.frames) {
                count++;
            }
        };

        TIMELINE_SELECTION_HANDLE_BOTH_CAPTURE(CountFunc);
        return count;
    }

    bool ContainsKeyframe(std::variant<KeyFrame<Vec2List>*, KeyFrame<float>*> frame) {
        bool contains = false;

        if (std::holds_alternative<KeyFrame<Vec2List>*>(frame)) {
            KeyFrame<Vec2List>* framePtr = std::get<KeyFrame<Vec2List>*>(frame);
            HandleTypeCapture<Vec2List>([&](const auto& val) {
                if (contains) return;
                for (auto* currFrame : val.frames) {
                    if (currFrame == framePtr) contains = true;
                }
            });
        } else {
            KeyFrame<float>* framePtr = std::get<KeyFrame<float>*>(frame);
            HandleTypeCapture<float>([&](const auto& val) {
                if (contains) return;
                for (auto* currFrame : val.frames) {
                    if (currFrame == framePtr) contains = true;
                }
            });
        }
        return contains;
    }
private:
    template <class T>
    void HandleTypeCapture(const std::function<void(const KeyFrameRowSelection<T>&)>& func) {
        for (const auto& selectionVariant : rowSelections) {
            if (std::holds_alternative<KeyFrameRowSelection<T>>(selectionVariant)) {
                func(std::get<KeyFrameRowSelection<T>>(selectionVariant));
            }
        }
    }

    void HandleAllCapture(const std::function<void(const KeyFrameRowSelection<Vec2List>&)>& vec2ListFuncPtr, const std::function<void(const KeyFrameRowSelection<float>&)>& floatFuncPtr) {
        for (const auto& selectionVariant : rowSelections) {
            if (std::holds_alternative<KeyFrameRowSelection<Vec2List>>(selectionVariant)) {
                vec2ListFuncPtr(std::get<KeyFrameRowSelection<Vec2List>>(selectionVariant));
            } else {
                floatFuncPtr(std::get<KeyFrameRowSelection<float>>(selectionVariant));
            }
        }
    }

    void HandleAll(void(* vec2ListFuncPtr)(const KeyFrameRowSelection<Vec2List>& val), void(* floatFuncPtr)(const KeyFrameRowSelection<float>& val)) {
        for (const auto& selectionVariant : rowSelections) {
            if (std::holds_alternative<KeyFrameRowSelection<Vec2List>>(selectionVariant)) {
                vec2ListFuncPtr(std::get<KeyFrameRowSelection<Vec2List>>(selectionVariant));
            } else {
                floatFuncPtr(std::get<KeyFrameRowSelection<float>>(selectionVariant));
            }
        }
    }
};

class Timeline {
public:
    constexpr static const float selectAreaSize = 0.4f;


    struct TimelineUpdateInfo {
        const Input& input;
        float deltaTime;
        Enums::DrawMode drawMode;
        ModelObject& modelObject;
        const std::vector<ModelObject*>& modelObjects;
        bool focusMode;
    };

    void Update(const TimelineUpdateInfo& info);
    void Render(const TimelineRenderInfo& info);

    void GUI(const TimelineGUIInfo& info);

    [[nodiscard]] bool IsFocused() const { return focused; }
    [[nodiscard]] bool IsPlaying() const { return playing; }

    TimelineSelection GenTimelineSelection();

    bool HasFloatLayer(const std::string& valLabel) {
        return animator->floatKeyFrameLayers.contains(valLabel);
    }

    void AddFloatLayer(const std::string& valLabel, float initVal) {
        animator->floatKeyFrameLayers[valLabel] = {KeyFrameLayer<float>()};
        UpdateFloat(valLabel, initVal);
    }

    void TryUpdateFloat(const std::string& valLabel, float val) {
        if (HasFloatLayer(valLabel)) UpdateFloat(valLabel, val);
    }

    void RemoveFloatLayer(const std::string& valLabel) {
        animator->floatKeyFrameLayers.erase(valLabel);
    }


    void UpdateFloat(const std::string& valLabel, float val) {
        animator->floatKeyFrameLayers[valLabel].layer.Insert({val, animator->currentTime});
    }

    void RenderOnionSkin(Mesh2D& plot, Enums::DrawMode drawMode);

    static float RowToHeight(int row) {
        return 0.9f - (float) row * 0.2f - selectAreaSize;
    }

    void SetActiveAnimator(Animator* animator) { this->animator = animator; }

    void OnActiveModelObjectChange() {
        playing = false;
    }

    explicit Timeline(const GLWindow& window) : scene({window.GetBufferWidth(), window.GetBufferHeight()}), animator(nullptr) {}


    static float RoundToTenth(float val);

private:
    float playbackSpeed = 1.0f;
    bool playing = false;
    bool focused = false, lastFocused = false, selecting = false, hasSelection = false;
    bool pingPong = false;
    bool keyFramePressed = false;
    Mesh2D canvas;
    RenderTarget scene;
    Rectangle guiRect;
    Animator* animator;
    Vec2 selectDragStart, selectDragEnd;
    Vec2 dragKeyFramesStart;
    TimelineSelection selection;
    bool dragging;
    float lastDragDiff;

    void TopToBottomLineAt(float x, glm::vec4 color, float width = 0.001f, bool trueTop = false);

};


#endif //SENIORRESEARCH_TIMELINE_H
