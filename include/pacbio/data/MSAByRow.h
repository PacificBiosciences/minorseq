// Copyright (c) 2016, Pacific Biosciences of California, Inc.
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

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <pacbio/data/ArrayRead.h>
#include <pacbio/data/MSARow.h>
#include <pacbio/data/QvThresholds.h>

namespace PacBio {
namespace Data {

struct MSAByRow
{
    MSAByRow() = default;

    MSAByRow(const std::vector<std::shared_ptr<Data::ArrayRead>>& reads)
    {
        for (const auto& r : reads)
            BeginEnd(*r);

        for (const auto& r : reads) {
            auto row = AddRead(*r);
            row.Read = r;
            Rows.emplace_back(std::move(row));
        }

        BeginPos += 1;
        EndPos += 1;
    }

    MSAByRow(const std::vector<Data::ArrayRead>& reads)
    {
        for (const auto& r : reads)
            BeginEnd(r);

        for (const auto& r : reads)
            Rows.emplace_back(AddRead(r));

        BeginPos += 1;
        EndPos += 1;
    }

    void BeginEnd(const Data::ArrayRead& read)
    {
        BeginPos = std::min(BeginPos, read.ReferenceStart());
        EndPos = std::max(EndPos, read.ReferenceEnd());
    }

    MSARow AddRead(const Data::ArrayRead& read)
    {
        MSARow row(EndPos - BeginPos);

        int pos = read.ReferenceStart() - BeginPos;
        assert(pos >= 0);

        std::string insertion;
        auto CheckInsertion = [&insertion, &row, &pos]() {
            if (insertion.empty()) return;
            row.Insertions[pos] = insertion;
            insertion = "";
        };

        for (const auto& b : read.Bases) {
            switch (b.Cigar) {
                case 'X':
                case '=':
                    CheckInsertion();
                    if (b.MeetQVThresholds(qvThresholds))
                        row.Bases[pos++] = b.Nucleotide;
                    else
                        row.Bases[pos++] = '-';
                    break;
                case 'D':
                    CheckInsertion();
                    row.Bases[pos++] = '-';
                    break;
                case 'I':
                    insertion += b.Nucleotide;
                    break;
                case 'P':
                    CheckInsertion();
                    break;
                case 'S':
                    CheckInsertion();
                    break;
                default:
                    throw std::runtime_error("Unexpected cigar " + std::to_string(b.Cigar));
            }
        }
        return row;
    }

    const Data::QvThresholds qvThresholds;
    int BeginPos = std::numeric_limits<int>::max();
    int EndPos = 0;
    std::vector<MSARow> Rows;
};
}
}  // ::PacBio::Juliet