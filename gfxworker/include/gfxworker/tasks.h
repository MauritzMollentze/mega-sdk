/**
 * (c) 2013 by Mega Limited, Auckland, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#pragma once

#include <string>
#include <vector>
#include <limits>
#include <memory>

#include "mega/filesystem.h"

namespace gfx {

class GfxSize
{
    int mWidth;
    int mHeight;
public:
    GfxSize() : mWidth(0), mHeight(0) {}
    GfxSize(int w, int h) : mWidth(w), mHeight(h) {}
    bool operator==(const GfxSize& other) const { return mWidth == other.mWidth && mHeight == other.mHeight; }
    bool operator!=(const GfxSize& other) const { return !(*this == other); }
    int w() const { return mWidth; }
    int h() const { return mHeight; }
    void setW(const int width) { mWidth = width; }
    void setH(const int height) { mHeight = height; }
    std::string toString() const { return std::to_string(mWidth) + "x" + std::to_string(mHeight); }
    static GfxSize fromString(const std::string& sizeStr);
};

enum class GfxSerializeVersion
{
    V_1 = 1,
    UNSUPPORTED
};

enum class GfxTaskProcessStatus;

constexpr const GfxSerializeVersion LATEST_SERIALIZE_VERSION =
        static_cast<GfxSerializeVersion>(static_cast<size_t>(GfxSerializeVersion::UNSUPPORTED) - 1);

struct GfxTask final
{
    std::string          Path;
    std::vector<GfxSize> Sizes;
};

/**
 * @brief Defines the possible result status of IGfxProcessor::process
 * This status can only be set during GfxTaskResult construction
 * This status is also used for the overall task lifecycle in the GfxServer:
 * - PENDING : when a new task is added to the list of pending tasks and has not
 *   yet been processed by IGfxProcessor::process
 * - SUCCESS/ERROR : once the task has been processed by IGfxProcessor::process
 * Note that there are no retrials
 */
enum class GfxTaskProcessStatus
{
    SUCCESS = 0,
    ERR     = 1,
    PENDING
};

struct GfxTaskResult final
{
    GfxTaskResult(std::vector<std::string>&& outputImages, const GfxTaskProcessStatus processStatus) 
        : ProcessStatus(processStatus)
        , OutputImages(std::move(outputImages)) {}

    GfxTaskProcessStatus     ProcessStatus;
    std::vector<std::string> OutputImages;
};

} //namespace gfx

