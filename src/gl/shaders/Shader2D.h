//
// Created by Tobiathan on 9/25/21.
//

#ifndef SENIORRESEARCH_SHADER2D_H
#define SENIORRESEARCH_SHADER2D_H


#include "Shader.h"

class Shader2D : public Shader {
public:
    Shader2D(const char *vertexShaderSource, const char *fragmentShaderSource);

    void SetModel(glm::mat4 model) { uniformModel.SetMat4(model); }

    static Shader2D Read(const char* vertexShaderPath, const char* fragmentShaderPath);
private:
    Uniform uniformModel;
};


#endif //SENIORRESEARCH_SHADER2D_H
