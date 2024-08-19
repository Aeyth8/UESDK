#pragma once

#include <sdk/Math.hpp>

#include "UObject.hpp"
#include "UActorComponent.hpp"

namespace sdk {
class USceneComponent : public UActorComponent {
public:
    static UClass* static_class();

public:
    void set_world_rotation(const glm::vec3& rotation, bool sweep = false, bool teleport = false);
    void add_world_rotation(const glm::vec3& rotation, bool sweep = false, bool teleport = false);
    void set_world_location(const glm::vec3& location, bool sweep = false, bool teleport = false);
    void add_world_offset(const glm::vec3& offset, bool sweep = false, bool teleport = false);
    void add_local_rotation(const glm::vec3& rotation, bool sweep = false, bool teleport = false);
    void set_local_transform(const glm::vec3& location, const glm::vec4& rotation, const glm::vec3& scale, bool sweep = false, bool teleport = false);

    glm::vec3 get_world_location();
    glm::vec3 get_world_rotation();

    bool attach_to(USceneComponent* parent, const std::wstring& socket_name = L"None", uint8_t attach_type = 0, bool weld = true);
    void set_hidden_in_game(bool hidden, bool propagate_to_children = true);

    bool is_visible();
    void set_visibility(bool visible, bool propagate_to_children = true);

    void detach_from_parent(bool maintain_world_position = true, bool call_modify = true);

    USceneComponent* get_attach_parent();

    sdk::TArray<sdk::FName> get_all_socket_names();
    glm::vec3 get_socket_location(const std::wstring& socket_name);
    glm::vec3 get_socket_rotation(const std::wstring& socket_name);
};
}