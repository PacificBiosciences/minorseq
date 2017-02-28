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

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

#include <pacbio/juliet/AminoAcidCaller.h>
#include <pacbio/juliet/AminoAcidTable.h>
#include <pacbio/juliet/Haplotype.h>
#include <pacbio/statistics/Fisher.h>
#include <pacbio/util/Termcolor.h>
#include <pbcopper/json/JSON.h>

namespace PacBio {
namespace Juliet {
using AAT = AminoAcidTable;

AminoAcidCaller::AminoAcidCaller(const std::vector<std::shared_ptr<Data::ArrayRead>>& reads,
                                 const ErrorEstimates& error, const JulietSettings& settings)
    : msaByRow_(reads)
    , msaByColumn_(msaByRow_)
    , error_(error)
    , targetConfig_(settings.TargetConfigUser)
    , verbose_(settings.Verbose)
    , mergeOutliers_(settings.MergeOutliers)
    , debug_(settings.Debug)
{

    CallVariants();
}

int AminoAcidCaller::CountNumberOfTests(const std::vector<TargetGene>& genes) const
{
    int numberOfTests = 0;
    for (const auto& gene : genes) {
        for (int i = gene.begin; i < gene.end - 2; ++i) {
            // Relative to gene begin
            const int ri = i - gene.begin;
            // Only work on beginnings of a codon
            if (ri % 3 != 0) continue;
            // Relative to window begin
            const int bi = i - msaByRow_.BeginPos;

            std::unordered_map<std::string, int> codons;
            for (const auto& nucRow : msaByRow_.Rows) {
                const auto& row = nucRow.Bases;
                // Read does not cover codon
                if (bi + 2 >= static_cast<int>(row.size()) || bi < 0) continue;
                if (row.at(bi + 0) == ' ' || row.at(bi + 1) == ' ' || row.at(bi + 2) == ' ')
                    continue;

                // Read has a deletion
                if (row.at(bi + 0) == '-' || row.at(bi + 1) == '-' || row.at(bi + 2) == '-')
                    continue;

                std::string codon = std::string() + row.at(bi) + row.at(bi + 1) + row.at(bi + 2);

                // Codon is bogus
                if (AAT::FromCodon.find(codon) == AAT::FromCodon.cend()) continue;

                codons[codon]++;
            }
            numberOfTests += codons.size();
        }
    }
    return numberOfTests;
}

std::string AminoAcidCaller::FindDRMs(const std::string& geneName,
                                      const std::vector<TargetGene>& genes,
                                      const int position) const
{
    std::string drmSummary;
    for (const auto& gene : genes) {
        if (geneName == gene.name) {
            for (const auto& drms : gene.drms) {
                if (std::find(drms.positions.cbegin(), drms.positions.cend(), position) !=
                    drms.positions.cend()) {
                    if (!drmSummary.empty()) drmSummary += " + ";
                    drmSummary += drms.name;
                }
            }
            break;
        }
    }
    return drmSummary;
};

void AminoAcidCaller::PhaseVariants()
{
    std::vector<std::pair<int, std::shared_ptr<VariantGene::VariantPosition>>> variantPositions;
    for (const auto& vg : variantGenes_) {
        for (const auto& pos_vp : vg.relPositionToVariant)
            if (pos_vp.second->IsVariant())
                variantPositions.emplace_back(
                    std::make_pair(vg.geneOffset + pos_vp.first * 3, pos_vp.second));
    }

    if (verbose_) {
        std::cerr << "Variant positions:";
        for (const auto& pos_var : variantPositions)
            std::cerr << " " << pos_var.first;
        std::cerr << std::endl;
    }
    std::vector<std::shared_ptr<Haplotype>> observations;
    std::vector<std::shared_ptr<Haplotype>> generators;

    // For each read
    for (const auto& row : msaByRow_.Rows) {

        // Get all codons for this row
        std::vector<std::string> codons;
        for (const auto& pos_var : variantPositions) {
            std::string codon;
            int local = pos_var.first - msaByRow_.BeginPos - 3;
            for (int i = 0; i < 3; ++i)
                codon += row.Bases.at(local + i);
            codons.emplace_back(std::move(codon));
        }

        // There are already haplotypes to compare against
        int miss = true;

        // Compare current row to existing haplotypes
        auto CompareHaplotypes = [&miss, &codons,
                                  &row](std::vector<std::shared_ptr<Haplotype>>& haplotypes) {
            for (auto& h : haplotypes) {
                // Don't trust if the number of codons differ.
                // That should only be the case if reads are not full-spanning.
                if (h->Codons.size() != codons.size()) {
                    continue;
                }
                bool same = true;
                for (size_t i = 0; i < codons.size(); ++i) {
                    if (h->Codons.at(i) != codons.at(i)) {
                        same = false;
                        break;
                    }
                }
                if (same) {
                    h->Names.push_back(row.Read->Name);
                    miss = false;
                    break;
                }
            }
        };

        CompareHaplotypes(generators);
        CompareHaplotypes(observations);

        // If row could not be collapsed into an existing haplotype
        if (miss) {
            auto h = std::make_shared<Haplotype>();
            h->Names = {row.Read->Name};
            h->SetCodons(std::move(codons));

            if (h->NoGaps)
                generators.emplace_back(std::move(h));
            else
                observations.emplace_back(std::move(h));
        }
    }

    // Move those haplotypes out of generators that have insufficient coverage
    auto f = [](const std::shared_ptr<Haplotype>& g) { return g->Size() >= 10; };
    auto p = std::stable_partition(generators.begin(), generators.end(), f);
    observations.insert(observations.end(), std::make_move_iterator(p),
                        std::make_move_iterator(generators.end()));
    generators.erase(p, generators.end());

    // Haplotype comparator, ascending
    auto HaplotypeComp = [](const std::shared_ptr<Haplotype>& a,
                            const std::shared_ptr<Haplotype>& b) { return a->Size() < b->Size(); };

    std::sort(generators.begin(), generators.end(), HaplotypeComp);
    std::sort(observations.begin(), observations.end(), HaplotypeComp);

    if (mergeOutliers_) {
        // Given the set of haplotypes clustered by identity, try collapsing
        // observations into generators.
        for (auto& hw : observations) {
            std::vector<double> probabilities;
            if (verbose_) std::cerr << *hw << std::endl;
            double genCov = 0;
            for (auto& hn : generators) {
                genCov += hn->Size();
                if (verbose_) std::cerr << *hn << " ";
                double p = 1;
                for (size_t a = 0; a < hw->Codons.size(); ++a) {
                    double p2 = transitions_.Transition(hn->Codons.at(a), hw->Codons.at(a));
                    if (verbose_) std::cerr << std::setw(15) << p2;
                    p *= p2;
                }
                if (verbose_) std::cerr << " = " << std::setw(15) << p << std::endl;
                probabilities.push_back(p);
            }

            double sum = std::accumulate(probabilities.cbegin(), probabilities.cend(), 0.0);
            std::vector<double> weight;
            for (size_t i = 0; i < generators.size(); ++i)
                weight.emplace_back(1.0 * generators[i]->Size() / genCov);

            std::vector<double> probabilityWeight;
            for (size_t i = 0; i < generators.size(); ++i)
                probabilityWeight.emplace_back(weight[i] * probabilities[i] / sum);

            double sumPW =
                std::accumulate(probabilityWeight.cbegin(), probabilityWeight.cend(), 0.0);

            for (size_t i = 0; i < generators.size(); ++i)
                generators[i]->SoftCollapses += 1.0 * hw->Size() * probabilityWeight[i] / sumPW;

            if (verbose_) std::cerr << std::endl;
        }
    }

    if (verbose_) {
        std::cerr << "#Haplotypes: " << generators.size() << std::endl;
        double counts = 0;
        for (auto& hn : generators)
            counts += hn->Size();
        std::cerr << "#Counts: " << counts << std::endl;

        for (size_t genNumber = 0; genNumber < generators.size(); ++genNumber) {
            auto& hn = generators.at(genNumber);
            std::cerr << hn->Size() / counts << "\t" << hn->Size() << "\t";
            size_t numCodons = hn->Codons.size();
            for (size_t i = 0; i < numCodons; ++i) {
                for (auto& kv : variantPositions.at(i).second->aminoAcidToCodons) {
                    for (auto& vc : kv.second) {
                        bool hit = hn->Codons.at(i) == vc.codon;
                        vc.haplotypeHit.push_back(hit);
                        if (hit) {
                            std::cerr << termcolor::red;
                        }
                    }
                }
                std::cerr << hn->Codons.at(i) << termcolor::reset << " ";
            }
            std::cerr << std::endl;
        }
    }
}

double AminoAcidCaller::Probability(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return 0.0;

    double p = 1;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] == '-' || b[i] == '-')
            p *= error_.deletion;
        else if (a[i] != b[i])
            p *= error_.substitution;
        else
            p *= error_.match;
    }
    return p;
};

void AminoAcidCaller::CallVariants()
{
    auto genes = targetConfig_.targetGenes;
    const size_t numExpectedMinors = targetConfig_.NumExpectedMinors();
    const bool hasExpectedMinors = numExpectedMinors > 0;

    const bool hasReference = !targetConfig_.referenceSequence.empty();
    // If no user config has been provided, use complete input region
    if (genes.empty()) {
        TargetGene tg(msaByRow_.BeginPos, msaByRow_.EndPos, "unknown", {});
        genes.emplace_back(tg);
    }

    VariantGene curVariantGene;
    std::string geneName;
    int geneOffset = 0;

    const auto SetNewGene = [this, &geneName, &curVariantGene, &geneOffset](
        const int begin, const std::string& name) {
        geneName = name;
        if (!curVariantGene.relPositionToVariant.empty())
            variantGenes_.emplace_back(std::move(curVariantGene));
        curVariantGene = VariantGene();
        curVariantGene.geneName = name;
        curVariantGene.geneOffset = begin;
        geneOffset = begin;
    };

    const int numberOfTests = CountNumberOfTests(genes);

    double truePositives = 0;
    double falsePositives = 0;
    double falseNegative = 0;
    double trueNegative = 0;
    auto MeasurePerformance = [&truePositives, &falsePositives, &falseNegative, &trueNegative, this,
                               &geneName](
        const TargetGene& tg, const std::pair<std::string, int>& codon_counts, const int& codonPos,
        const int& i, const double& p, const int& coverage) {
        const char aminoacid = AAT::FromCodon.at(codon_counts.first);
        auto Predictor = [&tg, &codonPos, &aminoacid, &codon_counts]() {
            if (!tg.minors.empty()) {
                for (const auto& minor : tg.minors) {
                    if (codonPos == minor.position && aminoacid == minor.aminoacid[0] &&
                        codon_counts.first == minor.codon) {
                        return true;
                    }
                }
            }
            return false;
        };
        double relativeCoverage = 1.0 * codon_counts.second / coverage;
        const bool variableSite = relativeCoverage < 0.8;
        if (variableSite) {
            if (p < alpha) {
                if (Predictor())
                    ++truePositives;
                else
                    ++falsePositives;
            } else {
                if (Predictor())
                    ++falseNegative;
                else
                    ++trueNegative;
            }
        }

        return variableSite;
    };

    for (const auto& gene : genes) {
        SetNewGene(gene.begin, gene.name);
        for (int i = gene.begin; i < gene.end - 2; ++i) {
            // Absolute reference position
            const int ai = i - 1;
            // Relative to gene begin
            const int ri = i - geneOffset;
            // Only work on beginnings of a codon
            if (ri % 3 != 0) continue;
            // Relative to window begin
            const int bi = i - msaByRow_.BeginPos;

            const int codonPos = 1 + (ri / 3);
            curVariantGene.relPositionToVariant.emplace(
                codonPos, std::make_shared<VariantGene::VariantPosition>());
            auto& curVariantPosition = curVariantGene.relPositionToVariant.at(codonPos);

            std::map<std::string, int> codons;
            int coverage = 0;
            for (const auto& nucRow : msaByRow_.Rows) {
                const auto& row = nucRow.Bases;
                const auto CodonContains = [&row, &bi](const char x) {
                    return (row.at(bi + 0) == x || row.at(bi + 1) == x || row.at(bi + 2) == x);
                };

                // Read does not cover codon
                if (bi + 2 > static_cast<int>(row.size()) || bi < 0) continue;
                if (CodonContains(' ')) continue;
                ++coverage;

                // Read has a deletion
                if (CodonContains('-')) continue;

                const auto codon = std::string() + row.at(bi) + row.at(bi + 1) + row.at(bi + 2);

                // Codon is bogus
                if (AAT::FromCodon.find(codon) == AAT::FromCodon.cend()) continue;

                codons[codon]++;
            }

            if (hasReference) {
                curVariantPosition->refCodon = targetConfig_.referenceSequence.substr(ai, 3);
                if (AAT::FromCodon.find(curVariantPosition->refCodon) == AAT::FromCodon.cend()) {
                    continue;
                }
                curVariantPosition->refAminoAcid = AAT::FromCodon.at(curVariantPosition->refCodon);
            } else {
                int max = -1;
                std::string argmax;
                for (const auto& codon_counts : codons) {
                    if (codon_counts.second > max) {
                        max = codon_counts.second;
                        argmax = codon_counts.first;
                    }
                }
                curVariantPosition->refCodon = argmax;
                if (AAT::FromCodon.find(curVariantPosition->refCodon) == AAT::FromCodon.cend()) {
                    continue;
                }
                curVariantPosition->refAminoAcid = AAT::FromCodon.at(curVariantPosition->refCodon);
            }

            for (const auto& codon_counts : codons) {
                if (AAT::FromCodon.at(codon_counts.first) == curVariantPosition->refAminoAcid)
                    continue;
                double p =
                    (Statistics::Fisher::fisher_exact_tiss(
                         codon_counts.second, coverage,
                         coverage * Probability(curVariantPosition->refCodon, codon_counts.first),
                         coverage) *
                     numberOfTests);

                if (p > 1) p = 1;

                const bool variableSite =
                    MeasurePerformance(gene, codon_counts, codonPos, ai, p, coverage);

                if (debug_ ||
                    (((hasExpectedMinors && variableSite) || !hasExpectedMinors) && p < alpha)) {
                    VariantGene::VariantPosition::VariantCodon curVariantCodon;
                    curVariantCodon.codon = codon_counts.first;
                    curVariantCodon.frequency = codon_counts.second / static_cast<double>(coverage);
                    curVariantCodon.pValue = p;
                    curVariantCodon.knownDRM = FindDRMs(geneName, genes, codonPos);

                    curVariantPosition->aminoAcidToCodons[AAT::FromCodon.at(codon_counts.first)]
                        .push_back(curVariantCodon);
                }
            }
            if (!curVariantPosition->aminoAcidToCodons.empty()) {
                curVariantPosition->coverage = coverage;
                for (int j = -3; j < 6; ++j) {
                    if (i + j >= msaByRow_.BeginPos && i + j < msaByRow_.EndPos) {
                        int abs = ai + j;
                        JSON::Json msaCounts;
                        msaCounts["rel_pos"] = j;
                        msaCounts["abs_pos"] = abs;
                        msaCounts["A"] = msaByColumn_[abs][0];
                        msaCounts["C"] = msaByColumn_[abs][1];
                        msaCounts["G"] = msaByColumn_[abs][2];
                        msaCounts["T"] = msaByColumn_[abs][3];
                        msaCounts["-"] = msaByColumn_[abs][4];
                        if (hasReference)
                            msaCounts["wt"] =
                                std::string(1, targetConfig_.referenceSequence.at(abs));
                        else
                            msaCounts["wt"] = std::string(
                                1, Data::TagToNucleotide(msaByColumn_[abs].MaxElement()));
                        curVariantPosition->msa.push_back(msaCounts);
                    }
                }
            }
        }
    }
    if (hasExpectedMinors) {
        double tpr = truePositives / numExpectedMinors;
        double fpr = falsePositives / (numberOfTests - numExpectedMinors);
        double acc = (truePositives + trueNegative) /
                     (truePositives + falsePositives + falseNegative + trueNegative);
        std::cerr << tpr << " " << fpr << " " << numberOfTests << " " << acc << " "
                  << falsePositives << std::endl;
    }
    if (!curVariantGene.relPositionToVariant.empty())
        variantGenes_.emplace_back(std::move(curVariantGene));
}

JSON::Json AminoAcidCaller::JSON()
{
    using JSON::Json;
    Json root;
    std::vector<Json> genes;
    for (const auto& v : variantGenes_) {
        Json j = v.ToJson();
        if (j.find("variant_positions") != j.cend()) genes.push_back(j);
    }
    root["genes"] = genes;

    return root;
}
}
}  // ::PacBio::Juliet
