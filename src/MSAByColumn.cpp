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

#include <numeric>
#include <vector>

#include <pacbio/data/ArrayRead.h>
#include <pacbio/data/MSAColumn.h>
#include <pacbio/data/QvThresholds.h>

#include <pacbio/data/MSAByColumn.h>

namespace PacBio {
namespace Data {
MSAByColumn::MSAByColumn(const MSAByRow& msaRows)
{
    beginPos = msaRows.BeginPos - 1;
    endPos = msaRows.EndPos - 1;
    counts.resize(msaRows.EndPos - msaRows.BeginPos);
    int pos = msaRows.BeginPos;
    for (auto& c : counts)
        c.refPos = pos++;

    for (const auto& row : msaRows.Rows) {
        int localPos = 0;
        for (const auto& c : row.Bases) {
            switch (c) {
                case 'A':
                case 'C':
                case 'G':
                case 'T':
                case '-':
                    counts.at(localPos)[c]++;
                case ' ':
                    ++localPos;
                    break;
                default:
                    throw std::runtime_error("Unexpected base " + std::string(1, c));
            }
        }
        for (const auto& ins : row.Insertions) {
            counts[ins.first].insertions[ins.second]++;
        }
    }
}
}  // namespace Data
}  // namespace PacBio
