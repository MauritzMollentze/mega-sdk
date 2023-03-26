/**
 * @file mega/filefingerprint.h
 * @brief Sparse file fingerprint
 *
 * (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
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

#include <array>

#include "types.h"

namespace mega {

struct MEGA_API InputStreamAccess
{
    virtual m_off_t size() = 0;
    virtual bool read(byte *, unsigned) = 0;
    virtual ~InputStreamAccess() { }
};


// sparse file fingerprint, including size and mtime
struct MEGA_API FileFingerprint : public Cacheable
{
    m_off_t size = -1;
    m_time_t mtime = 0;
    std::array<int32_t, 4> crc{};

    // if true, represents actual file data
    // if false, is constructed from node ctime/key
    bool isvalid = false;

    // Generates a fingerprint by iterating through`fa`
    bool genfingerprint(FileAccess* fa, bool ignoremtime = false);

    // Generates a fingerprint by iterating through `is`
    bool genfingerprint(InputStreamAccess* is, m_time_t cmtime, bool ignoremtime = false);

    void serializefingerprint(string* d) const;
    int unserializefingerprint(string* d);

    FileFingerprint() = default;

    FileFingerprint(const FileFingerprint&);
    FileFingerprint& operator=(const FileFingerprint& other);

    bool serialize(string* d) override;
    static unique_ptr<FileFingerprint> unserialize(const char*& ptr, const char* end);

    // convenience function for clear comparisons etc, referring to (this) base class
    const FileFingerprint& fingerprint() const { return *this; }

    string fingerprintDebugString() const;
};

// orders transfers by file fingerprints, ordered by size / mtime / sparse CRC
struct MEGA_API FileFingerprintCmp
{
    bool operator()(const FileFingerprint* a, const FileFingerprint* b) const;
    bool operator()(const FileFingerprint& a, const FileFingerprint& b) const;
};

bool operator==(const FileFingerprint& lhs, const FileFingerprint& rhs);
bool operator!=(const FileFingerprint& lhs, const FileFingerprint& rhs);


} // mega
