// Copyright (c) 2011-2016, Pacific Biosciences of California, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted (subject to the limitations in the
// disclaimer below) provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//  * Neither the name of Pacific Biosciences nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY PACIFIC
// BIOSCIENCES AND ITS CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL PACIFIC BIOSCIENCES OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

// Author: Armin Töpfer

#pragma once

#include <map>
#include <string>
#include <vector>

#include <pacbio/data/ArrayRead.h>
#include <pacbio/data/MSAColumn.h>

namespace PacBio {
namespace Data {
/// Multiple sequence alignment containing counts
class MSA
{
private:
    using MsaVec = std::vector<MSAColumn>;

public:
    using MsaIt = MsaVec::iterator;
    using MsaItConst = MsaVec::const_iterator;

public:
    MSA(const std::vector<Data::ArrayRead>& reads);
    MSA(const std::vector<Data::ArrayRead>& reads, const boost::optional<uint8_t> qualQv,
        const boost::optional<uint8_t> delQv, const boost::optional<uint8_t> subQv,
        const boost::optional<uint8_t> insQv);
    MSA(const std::vector<Data::ArrayRead>& reads, const MSA& prior);

public:
    /// Parameter is an index in ABSOLUTE reference space
    MSAColumn operator[](int i) const { return counts[i - beginPos]; }
    /// Parameter is an index in ABSOLUTE reference space
    MSAColumn& operator[](int i) { return counts[i - beginPos]; }

    bool has(int i) { return i >= beginPos && i < endPos; }

    // clang-format off
    MsaIt      begin()        { return counts.begin();  }
    MsaIt      end()          { return counts.end();    }
    MsaItConst begin() const  { return counts.begin();  }
    MsaItConst end() const    { return counts.end();    }
    MsaItConst cbegin() const { return counts.cbegin(); }
    MsaItConst cend() const   { return counts.cend();   }
    // clang-format on

public:
    MsaVec counts;
    int beginPos = std::numeric_limits<int>::max();
    int endPos = 0;

private:
    void BeginEnd(const std::vector<Data::ArrayRead>& reads);
    void FillCounts(const std::vector<ArrayRead>& reads);
    void FillCounts(const std::vector<ArrayRead>& reads, const boost::optional<uint8_t> qualQv,
                    const boost::optional<uint8_t> delQv, const boost::optional<uint8_t> subQv,
                    const boost::optional<uint8_t> insQv);
    void FillCounts(const std::vector<ArrayRead>& reads, const MSA& prior);
};
}  // namespace Data
}  // namespace PacBio
