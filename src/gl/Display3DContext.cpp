//
// Created by Tobiathan on 11/19/21.
//

#include "Display3DContext.h"
#include "../vendor/glm/ext/matrix_transform.hpp"

const float sensitivity = 4.0f;

void Display3DContext::Update(Display3DContextUpdateInfo info) {
    const auto& [input, camera, guiRect, focused] = info;

    if (!focused) return;

    Vec2 mousePos = Util::NormalizeToRectNPFlipped(input.GetMouse(), guiRect);

    if (Util::VecIsNormalizedNP(mousePos) && input.mouseDown) {
        Vec2 diff = mousePos - Util::NormalizeToRectNPFlipped(input.GetLastMouse(), guiRect);

        float angleDiffX = -diff.x * sensitivity;
        float angleDiffY = diff.y * sensitivity * (guiRect.height / guiRect.width);
        float newAngleY = Camera::ClampedPitch(camera.GetPitch() + angleDiffY);
        angleDiffY = newAngleY - camera.GetPitch();


        camera.SetPos(glm::rotate(glm::mat4(1.0f), angleDiffX, {0.0f, 1.0f, 0.0f}) * Vec4(camera.GetPos(), 1.0f));
        camera.SetYaw(camera.GetYaw() - angleDiffX);


        camera.SetPos(glm::rotate(glm::mat4(1.0f), angleDiffY, camera.GenRight()) * Vec4(camera.GetPos(), 1.0f));
        camera.SetPitch(camera.GetPitch() + angleDiffY);
    }
}