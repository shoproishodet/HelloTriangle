#ifndef HELLOTRIANGLE_QUEUEFAMILYINDICES_H
#define HELLOTRIANGLE_QUEUEFAMILYINDICES_H

#include <optional>
#include <cstdint>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

#endif //HELLOTRIANGLE_QUEUEFAMILYINDICES_H
