//
// Created by Tobiathan on 11/7/21.
//

#include "CrossSectional.h"
#include "Sampler.h"
#include "CrossSectionTracer.h"
#include "../gl/MeshUtil.h"

void CrossSectional::HyperParameterUI(const UIInfo& info) {
    const auto BindUIMeshUpdate = [&]() {
        if (ImGui::IsItemActive())
            UpdateMesh();
    };
    ImGui::SliderInt("count-per-ring", &countPerRing, 3, 40);
    BindUIMeshUpdate();

    ImGui::SliderFloat("sample-length", &sampleLength, 0.01f, 0.5f);
    BindUIMeshUpdate();



    if (ImGui::Checkbox("wrap-start", &wrapStart)) {
        UpdateMesh();
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("wrap-end", &wrapEnd)) {
        UpdateMesh();
    }
}

void CrossSectional::UpdateMesh() {

    segments.clear();

    if (boundPoints.size() >=2 && centralPoints.size() >= 2) {
        const auto sampled = Sampler::DumbSample(boundPoints, sampleLength);
        const auto sampled2 = Sampler::DumbSample(centralPoints, sampleLength);

        const CrossSectionTracer::CrossSectionTraceData traceData = GenTraceData();
        const auto tempSegments = CrossSectionTracer::TraceSegments(sampled, sampled2, traceData);
        mesh.Set(CrossSectionTracer::Inflate(tempSegments, traceData));
        segments.insert(segments.end(), tempSegments.begin(), tempSegments.end());
    } else {
        mesh.Set(MeshUtil::Empty());
    };
}

void CrossSectional::RenderSelf2D(RenderInfo2D renderInfo) {
    renderInfo.plot.AddLines(centralPoints, centralColor);
    renderInfo.plot.AddLines(boundPoints, {0.0f, 0.0f, 0.0f, 1.0f});
}

void CrossSectional::RenderGizmos2D(RenderInfo2D renderInfo) {
    if (renderInfo.drawMode == Enums::MODE_GRAPH_Y) FunctionalAngleGizmo(renderInfo, centralPoints);

    if (segments.empty()) return;

    float col = 0.5f;
    for (const auto& segment : segments) {
        renderInfo.plot.AddLines({segment.p1, segment.p2}, {col, col, col, 1.0f}, 0.001f);
    }
}

void CrossSectional::AuxParameterUI(const UIInfo& info) {
    if (ImGui::CollapsingHeader("Aux")) {
        ImGui::ColorEdit3("model-color", (float *) &color);
        ImGui::ColorEdit3("bound-color", (float *) &boundColor);
        ImGui::ColorEdit3("central-color", (float *) &centralColor);
        ImGui::SliderFloat3("translate", (float *) &modelTranslation, -5.f, 5.f);
    }
}

void CrossSectional::ModeSetUI(Enums::DrawMode& drawMode) {
    ModeSet("Bounds", Enums::DrawMode::MODE_PLOT, boundPoints, drawMode);
    ModeSet("Central-Trace", Enums::DrawMode::MODE_GRAPH_Y, centralPoints, drawMode);
}

void CrossSectional::ClearAll() {
    boundPoints.clear();
    centralPoints.clear();
    DiffPoints(Enums::MODE_PLOT);
    DiffPoints(Enums::MODE_GRAPH_Y);
}

std::vector<glm::vec3> Convert (const std::vector<glm::vec2>& vec) {
    std::vector<glm::vec3> points3D;
    points3D.reserve(vec.size());
    for (const auto& point : vec) {
        points3D.emplace_back(glm::vec3(point.x, point.y, 0.0f));
    }
    return points3D;
}



void CrossSectional::RenderGizmos3D(RenderInfo3D renderInfo) {
    if (centralPoints.size() > 2) {
        centralAxisMesh.Set(MeshUtil::PolyLine(Convert(Sampler::DumbSample(centralPoints, 0.075f))));
        lineLight.SetColor(centralColor);
        lineLight.Apply(renderInfo.shader3D);
        centralAxisMesh.Render();
    }

    if (boundPoints.size() > 2) {
        centralAxisMesh.Set(MeshUtil::PolyLine(Convert(Sampler::DumbSample(boundPoints, 0.075f))));
        lineLight.SetColor(boundColor);
        lineLight.Apply(renderInfo.shader3D);
        centralAxisMesh.Render();
    }
}

CrossSectionTracer::CrossSectionTraceData CrossSectional::GenTraceData() {
    return {countPerRing, wrapStart, wrapEnd};
}

std::vector<glm::vec2>& CrossSectional::GetPointsRefByMode(Enums::DrawMode drawMode) {
    if (drawMode == Enums::MODE_GRAPH_Y) return centralPoints;
    return boundPoints;
}

Enums::LineType CrossSectional::LineTypeByMode(Enums::DrawMode drawMode) {
    if (drawMode == Enums::MODE_PLOT) return Enums::POLYLINE;
    return Enums::PIECEWISE;
}

std::tuple<std::vector<glm::vec3>, std::vector<GLuint>> CrossSectional::GenMeshTuple() {
    if (boundPoints.size() >=2 && centralPoints.size() >= 2) {
        const auto sampled = Sampler::DumbSample(boundPoints, sampleLength);
        const auto sampled2 = Sampler::DumbSample(centralPoints, sampleLength);

        const CrossSectionTracer::CrossSectionTraceData traceData = GenTraceData();
        return CrossSectionTracer::Trace(sampled, sampled2, traceData);
    }

    return MeshUtil::Empty();
}
