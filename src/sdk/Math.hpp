#pragma once

#include <algorithm>
#include <nlohmann/json.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_major_storage.hpp>
#include <glm/gtx/compatibility.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
using Vector3d = glm::dvec3;
using Vector4d = glm::dvec4;
using Matrix3x3f = glm::mat3x3;
using Matrix3x4f = glm::mat3x4;
using Matrix4x4f = glm::mat4x4;
using Matrix4x4d = glm::dmat4x4;

namespace utility::math {
using namespace glm;

static vec3 euler_angles(const glm::mat4& rot);
static float fix_angle(float ang);
static void fix_angles(const glm::vec3& angles);
static float clamp_pitch(float ang);

static vec3 euler_angles_from_steamvr(const glm::mat4& rot) {
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
    glm::extractEulerAngleYXZ(rot, yaw, pitch, roll);

    return { pitch, -yaw, -roll };
}

static vec3 euler_angles_from_steamvr(const glm::quat& q) {
    const auto m = glm::mat4{q};
    return euler_angles_from_steamvr(m);
}

static vec3 euler_angles_from_ue4(const glm::quat q) {
    const auto m = glm::mat4{q};

    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
    glm::extractEulerAngleYZX(m, yaw, roll, pitch);

    return { pitch, yaw, roll };
}

static glm::quat glm_to_ue4(const glm::quat q) {
    return glm::quat{ -q.w, -q.z, q.x, q.y };
}

static glm::quat ue4_to_glm(const glm::quat q) {
    return glm::quat{ -q.w, q.y, q.z, -q.x };
}

static vec3 glm_to_ue4(const glm::vec3 v) {
    return vec3{ -v.z, v.x, v.y };
}

static vec3 ue4_to_glm(const glm::vec3 v) {
    return vec3{ v.y, v.z, -v.x };
}

static float fix_angle(float ang) {
    auto angDeg = glm::degrees(ang);

    while (angDeg > 180.0f) {
        angDeg -= 360.0f;
    }

    while (angDeg < -180.0f) {
        angDeg += 360.0f;
    }

    return glm::radians(angDeg);
}

static void fix_angles(glm::vec3& angles) {
    angles[0] = fix_angle(angles[0]);
    angles[1] = fix_angle(angles[1]);
    angles[2] = fix_angle(angles[2]);
}

float clamp_pitch(float ang) {
    return std::clamp(ang, glm::radians(-89.0f), glm::radians(89.0f));
}

static glm::mat4 remove_y_component(const glm::mat4& mat) {
    // Remove y component and normalize so we have the facing direction
    const auto forward_dir = glm::normalize(Vector3f{ mat[2].x, 0.0f, mat[2].z });

    return glm::rowMajor4(glm::lookAtLH(Vector3f{}, Vector3f{ forward_dir }, Vector3f(0.0f, 1.0f, 0.0f)));
}

static quat to_quat(const vec3& v) {
    const auto mat = glm::rowMajor4(glm::lookAtLH(Vector3f{0.0f, 0.0f, 0.0f}, v, Vector3f{0.0f, 1.0f, 0.0f}));

    return glm::quat{mat};
}

static quat flatten(const quat& q) {
    const auto forward = glm::normalize(glm::quat{q} * Vector3f{ 0.0f, 0.0f, 1.0f });
    const auto flattened_forward = glm::normalize(Vector3f{forward.x, 0.0f, forward.z});
    return utility::math::to_quat(flattened_forward);
}

// Obtains the forward dir, and isolates the pitch component
static quat pitch_only(const quat& q) {
    const auto forward = glm::normalize(q * Vector3f{ 0.0f, 0.0f, 1.0f });
    const auto pitch = glm::asin(forward.y);
    return glm::quat(glm::vec3{ pitch, 0.0f, 0.0f });
}

// Euler must be in degrees
static glm::mat4 ue_rotation_matrix(const glm::vec3& rot) {
    const auto radyaw = glm::radians(rot.y);
    const auto radpitch = glm::radians(rot.x);
    const auto radroll = glm::radians(rot.z);

    const auto sp = glm::sin(radpitch);
    const auto sy = glm::sin(radyaw);
    const auto sr = glm::sin(radroll);
    const auto cp = glm::cos(radpitch);
    const auto cy = glm::cos(radyaw);
    const auto cr = glm::cos(radroll);

    return glm::mat4 {
        cp * cy, cp * sy, sp, 0,
        sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp, 0,
        -(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp, 0,
        0, 0, 0, 1
    };
}

// Euler must be in degrees
static glm::mat4 ue_inverse_rotation_matrix(const glm::vec3& rot) {
    const auto radyaw = glm::radians(rot.y);
    const auto radpitch = glm::radians(rot.x);
    const auto radroll = glm::radians(rot.z);

    return glm::mat4 {
        glm::mat4 {
            1, 0, 0, 0,
            0, glm::cos(radroll), glm::sin(radroll), 0,
            0, -glm::sin(radroll), glm::cos(radroll), 0,
            0, 0, 0, 1
        } *
        glm::mat4 {
            glm::cos(radpitch), 0, -glm::sin(radpitch), 0,
            0, 1, 0, 0,
            glm::sin(radpitch), 0, glm::cos(radpitch), 0,
            0, 0, 0, 1
        } *
        glm::mat4 {
            glm::cos(radyaw), -glm::sin(radyaw), 0, 0,
            glm::sin(radyaw), glm::cos(radyaw), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        }
    };
};

// Input is UE format rotation matrix
static glm::vec3 ue_euler_from_rotation_matrix(const glm::mat4& m) {
    const auto& x_axis = *(glm::vec3*)&m[0];
    const auto& y_axis = *(glm::vec3*)&m[1];
    const auto& z_axis = *(glm::vec3*)&m[2];

    auto rotator = glm::vec3{
        glm::degrees(glm::atan2(x_axis.z, glm::sqrt((x_axis.x * x_axis.x) + (x_axis.y * x_axis.y)))),
        glm::degrees(glm::atan2(x_axis.y, x_axis.x)),
        0.0f
    };

    glm::mat4 rotation_matrix = ue_rotation_matrix(rotator);
    glm::vec3 sy_axis = glm::vec3(rotation_matrix[1]);

    // Roll
    rotator.z = glm::degrees(glm::atan2(glm::dot(z_axis, sy_axis), glm::dot(y_axis, sy_axis)));

    return rotator;
}

static nlohmann::json to_json(const glm::vec3& v) {
    return nlohmann::json{ { "x", v.x }, { "y", v.y }, { "z", v.z } };
}

static nlohmann::json to_json(const glm::vec4& v) {
    return nlohmann::json{ { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } };
}

static nlohmann::json to_json(const glm::quat& q) {
    return nlohmann::json{ { "x", q.x }, { "y", q.y }, { "z", q.z }, { "w", q.w } };
}

static glm::vec3 from_json_vec3(const nlohmann::json& j) {
    glm::vec3 result{};

    if (j.contains("x") && j["x"].is_number()) {
        result.x = j["x"].get<float>();
    }

    if (j.contains("y") && j["y"].is_number()) {
        result.y = j["y"].get<float>();
    }

    if (j.contains("z") && j["z"].is_number()) {
        result.z = j["z"].get<float>();
    }

    return result;
}

static glm::vec4 from_json_vec4(const nlohmann::json& j) {
    glm::vec4 result{};

    if (j.contains("x") && j["x"].is_number()) {
        result.x = j["x"].get<float>();
    }

    if (j.contains("y") && j["y"].is_number()) {
        result.y = j["y"].get<float>();
    }

    if (j.contains("z") && j["z"].is_number()) {
        result.z = j["z"].get<float>();
    }

    if (j.contains("w") && j["w"].is_number()) {
        result.w = j["w"].get<float>();
    }

    return result;
}

static glm::quat from_json_quat(const nlohmann::json& j) {
    glm::quat result{};

    if (j.contains("x") && j["x"].is_number()) {
        result.x = j["x"].get<float>();
    }

    if (j.contains("y") && j["y"].is_number()) {
        result.y = j["y"].get<float>();
    }

    if (j.contains("z") && j["z"].is_number()) {
        result.z = j["z"].get<float>();
    }

    if (j.contains("w") && j["w"].is_number()) {
        result.w = j["w"].get<float>();
    }

    return result;
}
}