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

#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <numeric>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <pacbio/data/MSAByColumn.h>
#include <pacbio/data/MSAByRow.h>
#include <pacbio/juliet/ErrorEstimates.h>
#include <pacbio/juliet/Haplotype.h>
#include <pacbio/juliet/JulietSettings.h>
#include <pacbio/juliet/TargetConfig.h>
#include <pacbio/juliet/TransitionTable.h>
#include <pacbio/juliet/VariantGene.h>
#include <pbcopper/json/JSON.h>

namespace PacBio {
namespace Juliet {
/// Given a MSA and p-values for each nucleotide of each position,
/// generate machine-interpretable and human-readable output about mutated
/// amino acids.
class AminoAcidCaller
{
public:
    AminoAcidCaller(const std::vector<std::shared_ptr<Data::ArrayRead>>& reads,
                    const ErrorEstimates& error, const JulietSettings& settings);

public:
    /// Generate JSON output of variant amino acids
    JSON::Json JSON();

public:
    void PhaseVariants();

private:
    static constexpr float alpha = 0.01;
    void CallVariants();
    int CountNumberOfTests(const std::vector<TargetGene>& genes) const;
    std::string FindDRMs(const std::string& geneName, const std::vector<TargetGene>& genes,
                         const int position) const;
    double Probability(const std::string& a, const std::string& b);

private:
    Data::MSAByRow msaByRow_;
    TransitionTable transitions_;

public:
    Data::MSAByColumn msaByColumn_;

private:
    std::vector<VariantGene> variantGenes_;
    std::vector<Haplotype> reconstructedHaplotypes_;
    const ErrorEstimates error_;
    const TargetConfig targetConfig_;
    const bool verbose_;
    const bool mergeOutliers_;
    const bool debug_;
};
}
}  // ::PacBio::Juliet