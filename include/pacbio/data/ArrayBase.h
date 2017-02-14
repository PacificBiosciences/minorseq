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

#include <boost/optional.hpp>

namespace PacBio {
namespace Data {

/// A single base in an ArrayRead with its associated qvs and cigar
struct ArrayBase
{
    ArrayBase(char cigar, char nucleotide, uint8_t qualQV, uint8_t subQV, uint8_t delQV,
              uint8_t insQV)
        : Cigar(cigar)
        , Nucleotide(nucleotide)
        , QualQV(qualQV)
        , DelQV(delQV)
        , SubQV(subQV)
        , InsQV(insQV)
        , ProbTrue(1 - pow(10, -1.0 * qualQV / 10.0))
        , ProbCorrectBase(1 - pow(10, -1.0 * subQV / 10.0))
        , ProbNoDeletion(1 - pow(10, -1.0 * delQV / 10.0))
        , ProbNoInsertion(1 - pow(10, -1.0 * insQV / 10.0))
    {
    }
    ArrayBase(char cigar, char nucleotide, uint8_t qualQV)
        : Cigar(cigar)
        , Nucleotide(nucleotide)
        , QualQV(qualQV)
        , ProbTrue(1 - pow(10, -1.0 * qualQV / 10.0))
    {
    }
    ArrayBase(char cigar, char nucleotide) : Cigar(cigar), Nucleotide(nucleotide) {}

    bool MeetQualQVThreshold(boost::optional<uint8_t> threshold) const
    {
        return !threshold || !QualQV || *QualQV >= *threshold;
    }
    bool MeetDelQVThreshold(boost::optional<uint8_t> threshold) const
    {
        return !threshold || !DelQV || *DelQV >= *threshold;
    }
    bool MeetSubQVThreshold(boost::optional<uint8_t> threshold) const
    {
        return !threshold || !SubQV || *SubQV >= *threshold;
    }
    bool MeetInsQVThreshold(boost::optional<uint8_t> threshold) const
    {
        return !threshold || !InsQV || *InsQV >= *threshold;
    }

    char Cigar;
    char Nucleotide;
    boost::optional<uint8_t> QualQV;
    boost::optional<uint8_t> DelQV;
    boost::optional<uint8_t> SubQV;
    boost::optional<uint8_t> InsQV;
    double ProbTrue = 0;
    double ProbCorrectBase = 0;
    double ProbNoDeletion = 0;
    double ProbNoInsertion = 0;
};
}  // namespace Data
}  // namespace PacBio
