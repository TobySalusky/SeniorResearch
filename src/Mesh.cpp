//
// Created by Tobiathan on 9/18/21.
//

#include "Mesh.h"
#include "Normals.h"

Mesh::Mesh() {
    VBO = 0;
    VAO = 0;
    IBO = 0;
    indexCount = 0;
}

Mesh::Mesh(GLfloat *vertices, GLuint *indices, GLuint numOfVertices, GLuint numOfIndices) : Mesh() {
    Init(vertices, indices, numOfVertices, numOfIndices);
}

Mesh::~Mesh() {
    ClearMesh();
}

void Mesh::Init(GLfloat *vertices, GLuint *indices, GLuint numOfVertices, GLuint numOfIndices) {
    indexCount = numOfIndices;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, nullptr, usageHint);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, nullptr, usageHint);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // must unbind after VAO

    Set(vertices, indices, numOfVertices, numOfIndices);
}

void Mesh::Render() const {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::ClearMesh() {

    if (IBO != 0) {
        glDeleteBuffers(1, &IBO);
        IBO = 0;
    }

    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }

    indexCount = 0;
}

void Mesh::Set(GLfloat *vertices, GLuint *indices, GLuint numOfVertices, GLuint numOfIndices) {

    const std::vector<GLfloat> data = Normals::Define(vertices, indices, numOfVertices, numOfIndices);

    indexCount = numOfIndices;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numOfIndices, indices, usageHint);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), &data[0], usageHint);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::Set(const std::vector<glm::vec3> &vertices, const std::vector<GLuint> &indices) {
    Set((GLfloat*) &vertices[0], (GLuint*) &indices[0], vertices.size() * 3, indices.size());
}

void Mesh::Set(const std::tuple<std::vector<glm::vec3>, std::vector<GLuint>> &tuple) {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    tie(vertices, indices) = tuple;
    Set(vertices, indices);
}
