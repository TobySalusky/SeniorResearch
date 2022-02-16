//
// Created by Tobiathan on 11/2/21.
//

#include "CrossSectionTracer.h"
#include "Intersector.h"
#include "../vendor/glm/geometric.hpp"
#include "../vendor/glm/fwd.hpp"
#include "../vendor/glm/ext/matrix_transform.hpp"
#include "../util/Util.h"
#include "../graphing/Function.h"
#include "LineAnalyzer.h"
#include "../animation/LineLerper.h"
#include "Sampler.h"

std::tuple<std::vector<glm::vec3>, std::vector<unsigned int>>
CrossSectionTracer::Trace(const std::vector<glm::vec2> &points, const std::vector<glm::vec2> &pathTrace, const CrossSectionTraceData& data, TopologyCorrector* outTopologyData) {
    return Inflate(TraceSegments(points, pathTrace, data), data, outTopologyData);
}

std::tuple<std::vector<glm::vec3>, std::vector<unsigned int>>
CrossSectionTracer::Inflate(const std::vector<Segment> &segments, const CrossSectionTraceData& data, TopologyCorrector* outTopologyData) {
    const int countPerRing = data.countPerRing;

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    bool hasCrossSection = data.crossSectionPoints.size() >= 3;
    Vec2List sampledCrossSection;
    if (hasCrossSection) {
        sampledCrossSection = Sampler::SampleTo(data.crossSectionPoints, data.countPerRing);
    }

    const auto CrossSectionPointToVec4 = [] (Vec2 vec) {
        return Vec4(0.0f, vec.y, vec.x, 1.0f);
    };

    for (const auto &segment : segments) {
        const auto midpoint = (segment.p1 + segment.p2) / 2.0f;
        const float radius = glm::length(midpoint - segment.p1);

        for (int i = 0; i < countPerRing; i++) {
            float angle = (float) M_PI * 2 * ((float) ((float) i * 1 / (float) countPerRing));
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f,0.0f,0.0f)); // TODO: split mat creation from loop (b/c constant angles)
            glm::vec4 vec = (hasCrossSection) ? CrossSectionPointToVec4(sampledCrossSection[i] * radius) : rot * Vec4(0, radius, 0.0f, 1.0f);

            const float leanAngle = Util::Angle(segment.p2 - segment.p1) + (float) M_PI_2;

            vec = glm::rotate(glm::mat4(1.0f), leanAngle, {0.0f, 0.0f, 1.0f}) * vec;

            vec.x += midpoint.x;

            vertices.emplace_back(glm::vec3(vec.x, vec.y + midpoint.y, vec.z));
        }
    }

    for (int ring = 0; ring < (int) segments.size() - 1; ring++) {

        for (int i = 0; i < countPerRing; i++) {
            // define point indices for quad
            unsigned int p1 = i + ring * countPerRing;
            unsigned int p2 = (i + 1) % countPerRing + ring * countPerRing;
            unsigned int p3 = p1 + countPerRing;
            unsigned int p4 = p2 + countPerRing;

            indices.insert(indices.end(), {
                    p1, p3, p2,
                    p2, p3, p4,
            });
        }
    }

    const int sideQuadCount = (int) (segments.size() - 1) * countPerRing;

    const auto WrapEnd = [&](unsigned int startIndex, bool reverse = false) {
        for (int i = 0; i < countPerRing - 2; i++) {
            if (!reverse) {
                indices.insert(indices.end(), {
                        startIndex, (startIndex + 1 + i), (startIndex + 2 + i),
                });
            } else {
                indices.insert(indices.end(), {
                        (startIndex + 2 + i), (startIndex + 1 + i), startIndex,
                });
            }
        }
    };

    int wrapStartPoints = 0, wrapEndPoints = 0;
    if (!segments.empty()) {
        if (data.wrapStart) {
            WrapEnd(0);
            wrapStartPoints = countPerRing;
        }
        if (data.wrapEnd) {
            WrapEnd((segments.size() - 1) * countPerRing, true);
            wrapEndPoints = countPerRing;
        }
    }

    if (outTopologyData != nullptr) *outTopologyData = {sideQuadCount, wrapStartPoints, wrapEndPoints};
    return {vertices, indices};
}

std::vector<CrossSectionTracer::Segment>
CrossSectionTracer::TraceSegments(const std::vector<glm::vec2> &points, const std::vector<glm::vec2> &pathTrace, const CrossSectionTraceData& data) {
    std::vector<Segment> segments;

    // TODO: optimize!!!

    const float arcLen = LineAnalyzer::FullLength(pathTrace);
    const float sampleLen = data.sampleLength;

    for (int step = 0; step < (int) (arcLen / sampleLen); step++) {
        bool hasP1 = false, hasP2 = false;
        glm::vec2 p1, p2;

        const Ray2D tangentRay = Function::GetRayAtLength(pathTrace, (float) step * sampleLen);
        const Vec2 dirVec = Util::Perpendicular(tangentRay.dir);
        const Vec2 pathPoint = tangentRay.origin;

        for (int i = 0; i < points.size() - 1; i++) {

            const auto intersectionPos = Intersector::RaySegment(pathPoint, dirVec, points[i], points[i + 1]);
            const auto intersectionNeg = Intersector::RaySegment(pathPoint, -dirVec, points[i], points[i + 1]);

            if (intersectionPos.has_value()) {
                if (!hasP1) {
                    p1 = intersectionPos.value();
                    hasP1 = true;
                } else {
                    if (glm::length(intersectionPos.value() - pathPoint) < glm::length(p1 - pathPoint)) {
                        p1 = intersectionPos.value();
                    }
                }
            }

            if (intersectionNeg.has_value()) {
                if (!hasP2) {
                    p2 = intersectionNeg.value();
                    hasP2 = true;
                } else {
                    if (glm::length(intersectionNeg.value() - pathPoint) < glm::length(p2 - pathPoint)) {
                        p2 = intersectionNeg.value();
                    }
                }
            }
        }

        if (hasP1 && hasP2) {
            // may not intersect with previous segment
            if (segments.empty() || !Intersector::Segment(p1, p2, segments.back().p1, segments.back().p2)) {
                segments.push_back({p1, p2});
            }
        }
    }


//    for (const auto &pathPoint : pathTrace) {
//        bool hasP1 = false, hasP2 = false;
//        glm::vec2 p1, p2;
//
//        const float angle = -Function::GetAverageRadians(pathTrace, pathPoint.x, 3);
//        const auto dirVec = Util::Polar(angle);
//
//        for (int i = 0; i < points.size() - 1; i++) {
//
//            const auto intersectionPos = Intersector::RaySegment(pathPoint, dirVec, points[i], points[i + 1]);
//            const auto intersectionNeg = Intersector::RaySegment(pathPoint, -dirVec, points[i], points[i + 1]);
//
//            if (intersectionPos.has_value()) {
//                if (!hasP1) {
//                    p1 = intersectionPos.value();
//                    hasP1 = true;
//                } else {
//                    if (glm::length(intersectionPos.value() - pathPoint) < glm::length(p1 - pathPoint)) {
//                        p1 = intersectionPos.value();
//                    }
//                }
//            }
//
//            if (intersectionNeg.has_value()) {
//                if (!hasP2) {
//                    p2 = intersectionNeg.value();
//                    hasP2 = true;
//                } else {
//                    if (glm::length(intersectionNeg.value() - pathPoint) < glm::length(p2 - pathPoint)) {
//                        p2 = intersectionNeg.value();
//                    }
//                }
//            }
//        }
//
//        if (hasP1 && hasP2) {
//            // may not intersect with previous segment
//            if (segments.empty() || !Intersector::Segment(p1, p2, segments.back().p1, segments.back().p2)) {
//                segments.push_back({p1, p2});
//            }
//        }
//    }

    return segments;
}

Vec2List CrossSectionTracer::AutoGenChordalAxis(const Vec2List &boundPoints, const float sampleLength) {

    if (boundPoints.size() < 2) return {};
    const float halfway = LineAnalyzer::FullLength(boundPoints) / 2.0f;

    int midPointIndex = -1;
    float sumLength = 0.0f;
    for (int i = 0; i < boundPoints.size() - 1; i++) {
        sumLength += glm::length(boundPoints[i + 1] - boundPoints[i]);
        if (sumLength >= halfway) {
            midPointIndex = i + 1;
            break;
        }
    }

    if (midPointIndex == -1 || midPointIndex == boundPoints.size() - 1) return {};

    Vec2List secondHalf = {};
    secondHalf.reserve(boundPoints.size() - midPointIndex - 1);
    for (size_t i = boundPoints.size() - 1; i > midPointIndex; i--) {
        secondHalf.emplace_back(boundPoints[i]);
    }

    return LineLerper::MorphPolyLine(std::vector(boundPoints.begin(), boundPoints.begin() + midPointIndex),
                                     secondHalf,
                                     0.5f, ceil(halfway * 2.0f / sampleLength));
}
