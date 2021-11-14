//
// Created by Tobiathan on 11/14/21.
//

#ifndef SENIORRESEARCH_KEYFRAMELAYER_H
#define SENIORRESEARCH_KEYFRAMELAYER_H

#include <vector>
#include "KeyFrame.h"
#include "../vendor/glm/vec2.hpp"
#include "../gl/Mesh2D.h"
#include "../Enums.h"
#include "../util/Util.h"


template <class T>
class KeyFrameLayer {
public:
    void RemoveAtTime(float time) {
        for (int i = 0; i < frames.size(); i++) {
            if (frames[i].time == time) {
                frames.erase(frames.begin() + i);
                return;
            }
        }
    }

    static float RowToHeight(int row) {
        return 0.9f - (float) row * 0.2f;
    }

    void Render(Mesh2D& canvas, int line, float time) {
        const auto SideToSideLine = [&](float height) {
            canvas.AddLines({{-1.0f, height}, {1.0f, height}}, {0.5f, 0.5f, 0.5f, 1.0f}, 0.003f);
            canvas.AddLines({{-1.0f, height}, {1.0f, height}}, {0.5f, 0.5f, 0.5f, 1.0f}, 0.003f);
        };

        if (!frames.empty()) {
            auto upper = std::upper_bound(frames.begin(), frames.end(), time,
                                          [](float value, const KeyFrame<T>& frame) {
                                              return value < frame.time;
                                          }
            );
            const KeyFrame<T>& frame1 = [&]{
                if (upper == frames.begin()) return upper[0];
                if (upper == frames.end()) return frames.back();
                return upper[-1];
            }();
            const KeyFrame<T>& frame2 = [&]{
                if (upper == frames.end()) return frames.back();
                return upper[0];
            }();

            float t = (time - frame1.time) / (frame2.time - frame1.time); // Linear

            for (int i = 0; i < frames.size(); i++) {
                const auto& frame = frames[i];
                bool selected = (frame.time == frame1.time || frame.time == frame2.time);

                canvas.AddLines({{frame.time, RowToHeight(line) - 0.1f}, {frame.time, RowToHeight(line) + 0.1f}}, {0.43f, 0.43f, 0.43f, 1.0f}, 0.003f);

                glm::vec4 color = [&]{
                    if (frame.time == frame1.time && frame.time == frame2.time) return glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
                    if (frame.time == frame1.time) return (glm::vec4(1.0f, 0.5f, 0.0f, 1.0f) * (1.0f - t) + glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * t);
                    if (frame.time == frame2.time) return (glm::vec4(1.0f, 0.5f, 0.0f, 1.0f) * t + glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * (1.0f - t));
                    return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                }();
                canvas.AddPolygonOutline({frame.time, RowToHeight(line)}, 0.03f, 5, color);
                if (i != frames.size() - 1) {
                    glm::vec2 p1 = glm::vec2(frame.time, RowToHeight(line) - 0.1f);
                    glm::vec2 p2 = glm::vec2(frames[i + 1].time, RowToHeight(line) + 0.1f);

                    bool onLine = (frame.time == frame1.time && frame.time != frame2.time);
                    glm::vec4 lineColor = onLine ? glm::vec4(0.85f, 1.0f, 0.85f, 1.0f) : glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);

                    const auto GetGraphPosAtT = [&](float t) {
                        return p1 + glm::vec2(t, Util::SinSmooth01(t)) * (p2 - p1);
                    };

                    {
                        int pCount = std::round((frames[i + 1].time - frame.time) * 10.0f) * 4 + 1;
                        std::vector<glm::vec2> vec;
                        vec.reserve(pCount);

                        for (int drawP = 0; drawP <= pCount; drawP++) {
                            vec.push_back(GetGraphPosAtT((float) drawP / (float) pCount));
                        }

                        canvas.AddLines(vec, lineColor, 0.003f);
                    }

                    if (onLine) {
                        canvas.AddPolygonOutline(GetGraphPosAtT(t), 0.013f, 6, lineColor);
                    }
                }
            }

        }
        SideToSideLine(RowToHeight(line) - 0.1f);
        SideToSideLine(RowToHeight(line) + 0.1f);
    }

    bool HasValue() {
        return !frames.empty();
    }

    T GetAnimatedVal(float time) {
        auto upper = std::upper_bound(frames.begin(), frames.end(), time,
          [](float value, const KeyFrame<T>& frame) {
              return value < frame.time;
          }
        );
        const KeyFrame<T>& frame1 = [&]{
            if (upper == frames.begin()) return upper[0];
            if (upper == frames.end()) return frames.back();
            return upper[-1];
        }();
        const KeyFrame<T>& frame2 = [&]{
            if (upper == frames.end()) return frames.back();
            return upper[0];
        }();

        float t = (time - frame1.time) / (frame2.time - frame1.time); // Linear
        t = Util::SinSmooth01(t);

        return KeyFrame<T>::GetAnimatedValueAtT(frame1, frame2, t);
    }

    void Insert(KeyFrame<T> frame) {
        const float time = frame.time;

        for (int i = 0; i < frames.size(); i++) {
            float otherTime = frames[i].time;
            if (time == otherTime) {
                frames[i] = frame;
            }  else if (time < otherTime) {
                frames.insert(frames.begin() + i, {frame});
            } else {
                continue;
            }
            return;
        }
        frames.push_back(frame);
    }

protected:
    struct KeyFrameSet {
        KeyFrame<T>& frame1;
        KeyFrame<T>& frame2;

        bool IsEither(const KeyFrame<T>& frame) const {
            return frame == frame1 || frame == frame2;
        }
    };
private:

    KeyFrameSet CurrKeyFrames() {
        auto upper = std::upper_bound(frames.begin(), frames.end(), time,
            [](float value, const KeyFrame<T>& frame) {
                return value < frame.time;
            }
        );
        const KeyFrame<T>& frame1 = [&]{
            if (upper == frames.begin()) return upper[0];
            if (upper == frames.end()) return frames.back();
            return upper[-1];
        }();
        const KeyFrame<T>& frame2 = [&]{
            if (upper == frames.end()) return frames.back();
            return upper[0];
        }();
        return {frame1, frame2};
    }

    std::vector<KeyFrame<T>> frames;
};


#endif //SENIORRESEARCH_KEYFRAMELAYER_H
