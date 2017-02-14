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
#include <exception>
#include <fstream>
#include <functional>
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

#include <pacbio/data/MSA.h>
#include <pacbio/juliet/ResistanceCaller.h>
#include <pbcopper/json/JSON.h>

#undef major  // gcc hack
namespace PacBio {
namespace Juliet {

ResistanceCaller::ResistanceCaller(const Data::MSA& msa)
    : msa_(msa), begin_(msa.beginPos), end_(msa.endPos)
{
    for (const auto& column : msa) {
        std::vector<VariantNucleotide> nucs;
        for (int j = 0; j < 4; ++j) {
            char curNuc = Data::TagToNucleotide(j);
            if (column.argMax == j) {
                nucs.emplace_back(curNuc);
            } else if (column.mask.at(j) != 0) {
                nucs.emplace_back(curNuc, std::round(column.Frequency(j) * 10000) / 10000.0,
                                  column.pValues.at(j));
            }
        }
        AddPosition(std::move(nucs));
    }
}

JSON::Json ResistanceCaller::JSON()
{
    using namespace PacBio::JSON;

    Json j;
    std::vector<Json> genes;
    Json curGene;
    std::string gene;
    int geneOffset;
    for (int i = begin_; i < end_; ++i) {
        if ((i + 1) % 3 != 0) continue;
        if (i + 2 >= end_) break;
        if (!(msa_[i].hit || msa_[i + 1].hit || msa_[i + 2].hit)) continue;

        const auto codons = CreateCodons(i);
        if (codons.empty()) continue;

        const auto aminoRef = AminoacidRef(i);

        auto SetNewGene = [&gene, &genes, &curGene, &geneOffset](const std::string& name,
                                                                 int begin) {
            gene = name;
            if (curGene.find("gene") != curGene.cend()) genes.push_back(std::move(curGene));
            curGene = Json();
            curGene["name"] = name;
            geneOffset = begin;
        };
        // Corrected index for 1-based reference
        const int ci = i + 1;
        if (ci > 1 && ci < 634 && gene != "5'LTR") {
            SetNewGene("5'LTR", 1);
        } else if (ci > 790 && ci < 1186 && gene != "p17") {
            SetNewGene("p17", 790);
        } else if (ci > 1186 && ci < 1879 && gene != "p24") {
            SetNewGene("p24", 1186);
        } else if (ci > 1879 && ci < 1921 && gene != "p2") {
            SetNewGene("p2", 1879);
        } else if (ci > 1921 && ci < 2086 && gene != "p7") {
            SetNewGene("p7", 1921);
        } else if (ci > 2086 && ci < 2134 && gene != "p1") {
            SetNewGene("p1", 2086);
        } else if (ci > 2134 && ci < 2292 && gene != "p6") {
            SetNewGene("p6", 2134);
        } else if (ci > 2253 && ci < 2550 && gene != "Protease") {
            SetNewGene("Protease", 2253);
        } else if (ci > 2550 && ci < 3870 && gene != "Reverse Transcriptase") {
            SetNewGene("Reverse Transcriptase", 2550);
        } else if (ci > 3870 && ci < 4230 && gene != "RNase") {
            SetNewGene("RNase", 3870);
        } else if (ci > 4230 && ci < 5096 && gene != "Integrase") {
            SetNewGene("Integrase", 4230);
        }

        const auto refCodon = CodonRef(i);
        std::stringstream line;
        Json variantPosition;
        variantPosition["ref_codon"] = refCodon;
        variantPosition["ref_amino_acid"] = std::string(1, aminoRef);
        variantPosition["ref_position"] = 1 + (ci + 1 - geneOffset) / 3;
        std::vector<Json> variants;
        bool first = true;
        bool hit = false;
        for (const auto& c : codons) {
            const auto cc = CodonString(c);
            const auto a = codonToAmino_.at(cc);
            bool isKnown = resistantCodon_.find(i + 4) != resistantCodon_.cend();
            if (a != aminoRef) {
                hit = true;
                Json variant;

                variant["amino_acid"] = std::string(1, a);
                variant["nucleotides"] = {std::string(1, cc[0]), std::string(1, cc[1]),
                                          std::string(1, cc[2])};
                variant["frequencies"] = {c[0].frequency, c[1].frequency, c[2].frequency};
                variant["p-values"] = {c[0].major ? 0 : c[0].pValue, c[1].major ? 0 : c[1].pValue,
                                       c[2].major ? 0 : c[2].pValue};
                variant["coverage"] = {msa_[i + 0].Coverage(), msa_[i + 1].Coverage(),
                                       msa_[i + 2].Coverage()};
                variant["known_drm"] = isKnown ? resistantCodon_.at(i + 4) : "";

                for (int j = -3; j < 6; ++j) {
                    if (i + j >= begin_ && i + j < end_) {
                        Json msaCounts;
                        msaCounts["rel_pos"] = j;
                        msaCounts["abs_pos"] = i + j;
                        msaCounts["A"] = msa_[i + j][0];
                        msaCounts["C"] = msa_[i + j][1];
                        msaCounts["G"] = msa_[i + j][2];
                        msaCounts["T"] = msa_[i + j][3];
                        msaCounts["-"] = msa_[i + j][4];
                        msaCounts["wt"] = std::string(1, ref_[i + j]);
                        variant["msa_counts"].push_back(msaCounts);
                    }
                }
                variants.push_back(variant);
            }
            variantPosition["variants"] = variants;
        }
        std::vector<Json> insertions;
        for (const auto& kv : msa_[i].insertionsPValues) {
            if (kv.first.size() % 3 == 0) {
                Json insertion;
                insertion["nucleotides"] = kv.first;
                insertion["p-values"] = kv.second;
                insertion["abundance"] = msa_[i].insertions[kv.first];
                std::string aa;
                for (size_t ii = 0; ii < kv.first.size(); ii += 3)
                    aa += codonToAmino_.at(kv.first.substr(ii, 3));
                insertion["amino_acid"] = aa;
                insertions.push_back(insertion);
            }
        }
        if (!insertions.empty()) variantPosition["insertions"] = insertions;
        if (hit) curGene["variant_positions"].push_back(variantPosition);
    }
    if (!gene.empty()) genes.push_back(std::move(curGene));
    if (!genes.empty()) j["genes"] = genes;
    return j;
}

void ResistanceCaller::HTML(std::ostream& out, const JSON::Json& j, bool onlyKnownDRMs,
                            bool details)
{
    auto strip = [](const std::string& input) -> std::string {
        std::string s = input;
        s.erase(std::remove(s.begin(), s.end(), '\"'), s.end());
        return s;
    };
    out << "<html>" << std::endl;
    out << "<head>" << std::endl;
    out << R"(
<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js"></script>
<script type="text/javascript">
$(document).ready(function() {
    $(".var").bind( "click", function( event ) {
        $(this).next().slideToggle(0);
});
});
</script>
<style>)"
        << std::endl;
    out << "<style>" << std::endl;
    out << R"(
body { font-family: arial }
table { border-collapse: collapse; margin-bottom: 20px; }
/*th { padding: 5px 5px 5px 5px; text-align: center; border-bottom: 1px solid #2d2d2d; }*/
tr:nth-child(1) { border: 1px solid #2d2d2d; background-color: #2d2d2d; color: white; }
tr:nth-child(2) { border-top: 1px solid #2d2d2d; border-left: 1px solid #2d2d2d; border-right: 1px solid #2d2d2d; }
tr:nth-child(3) { border-left: 1px solid #2d2d2d; border-right: 1px solid #2d2d2d; }
tr:nth-child(3) th { padding: 5px 5px 5px 5px; text-align: center; border-bottom: 1px solid #2d2d2d; }
tr:nth-child(2) th:nth-child(2) { border-right: 1px solid black; border-left: 1px solid black; }
tr:nth-child(3) th:nth-child(3) { border-right: 1px solid black; border-left: 1px solid black; }
td { padding: 15px 5px 15px 5px; text-align: center; border-bottom: 1px solid white; }
#hitC0 { color: #fff; }
#hitC1 { color: #fff; }
#hitC2 { color: #fff; }
#hitF0 { color: #fff; }
#hitF1 { color: #fff; }
#hitF2 { color: #fff; }
#hitP0 { color: #fff; }
#hitP1 { color: #fff; }
#hitP2 { color: #fff; }
table td:nth-child(1) { background-color:#ddd; border-right: 1px dashed #ccc; }
table td:nth-child(2) { background-color:#eee; border-right: 1px solid #ddd; }
table td:nth-child(3) { background-color:#fff; border-right: 1px solid #ddd; font-weight: bold;}
table td:nth-child(4) { background-color:#eee; border-right: 1px dashed #ccc;  }
table td:nth-child(5) { background-color: #ddd; }
table td:nth-child(6) { background-color: #ddd; }
table td:nth-child(7) { background-color: #ddd; border-right: 1px dashed #bbb; }
table td:nth-child(8) { background-color: #ccc; }
table td:nth-child(9) { background-color: #ccc; }
table td:nth-child(10) { background-color: #ccc; border-right: 1px dashed #aaa; }
table td:nth-child(11) { background-color: #bbb; }
table td:nth-child(12) { background-color: #bbb; }
table td:nth-child(13) { background-color: #bbb; }
table td:nth-child(14) { background-color: #aaa; }
table td:nth-child(15) { background-color: #aaa; }
table td:nth-child(16) { background-color: #aaa; }
table td:nth-child(17) { background-color: #999; color: #fff600 }
tr:not(.msa):hover td { background-color: #ff5e5e; }
.msa table tr:hover td { background-color: #42bff4; }
.top table { background-color:white; border:0; }
.top table td { background-color:white; border:0; border-bottom: 1px solid gray; font-weight: normal}
.top table tr { border:0; }
.top table th { border:0; }
.msa { display:none; }
)" << std::endl;
    out << "</style>" << std::endl;
    out << "</head>" << std::endl;
    out << "<body>" << std::endl;

    if (j.find("genes") == j.cend() || j["genes"].is_null()) return;
    for (const auto& gene : j["genes"]) {
        out << "<table class=\"top\">" << std::endl;
        out << R"(
<col width="60px"/>
<col width="40px"/>
<col width="40px"/>
<col width="40px"/>
<col width="30px"/>
<col width="30px"/>
<col width="30px"/>
<col width="60px"/>
<col width="60px"/>
<col width="60px"/>
<col width="120px"/>
<col width="120px"/>
<col width="120px"/>
<col width="80px"/>
<col width="80px"/>
<col width="80px"/>
<col width="180px"/>
<tr>
<th colspan="17">)";
        out << gene["name"];
        out << R"(</th>
</tr>
<tr>
<th colspan="2">Reference</th>
<th colspan="1">HXB2</th>
<th colspan="14">Sample</th>
</tr>
<tr>
<th>Codon</th>
<th>AA</th>
<th>Pos</th>
<th>AA</th>
<th colspan="3">Codon</th>
<th colspan="3">Frequency</th>
<th colspan="3">p-value</th>
<th colspan="3">Coverage</th>
<th colspan="1">DRM</th>
</tr>)" << std::endl;

        for (auto& variantPosition : gene["variant_positions"]) {
            std::stringstream line;
            const std::string refCodon = strip(variantPosition["ref_codon"]);
            line << "<tr class=\"var\">\n"
                 << "<td>" << refCodon << "</td>\n<td>" << strip(variantPosition["ref_amino_acid"])
                 << "</td>\n"
                 << "<td>" << variantPosition["ref_position"] << "</td>";
            std::string prefix = line.str();
            line.str("");
            bool first = true;
            if (variantPosition.find("insertions") != variantPosition.cend())
                for (auto& insertion : variantPosition["insertions"]) {
                    out << "<tr style=\"\">\n"
                        << "<td colspan=\"2\" style=\"background-color: "
                           "orange\">Insertion</td>\n"
                        << "<td style=\"background-color: white; font-weight: bold\">"
                        << variantPosition["ref_position"] << "</td>"
                        << "<td colspan=\"1\" style=\"background-color: orange; "
                           "font-weight: "
                           "normal\">"
                        << strip(insertion["amino_acid"]) << "</td>"
                        << "<td colspan=\"3\" style=\"background-color: orange; "
                           "font-weight: "
                           "normal\">"
                        << strip(insertion["nucleotides"]) << "</td>"
                        << "</tr>";
                }
            for (auto& variant : variantPosition["variants"]) {
                bool isKnown = !strip(variant["known_drm"]).empty();
                if ((onlyKnownDRMs && isKnown) || !onlyKnownDRMs) {
                    line << "<td>" << strip(variant["amino_acid"]) << "</td>\n";
                    // line << "[";
                    bool mutated[]{refCodon[0] != strip(variant["nucleotides"][0])[0],
                                   refCodon[1] != strip(variant["nucleotides"][1])[0],
                                   refCodon[2] != strip(variant["nucleotides"][2])[0]};
                    for (int j = 0; j < 3; ++j) {
                        line << "<td";
                        if (mutated[j]) line << " id=\"hitC" + std::to_string(j) + "\" ";
                        line << ">";
                        line << strip(variant["nucleotides"][j]);
                        line << "</td>\n";
                    }
                    for (int j = 0; j < 3; ++j) {
                        line << "<td";
                        if (mutated[j]) line << " id=\"hitF" + std::to_string(j) + "\" ";
                        line << ">";
                        line << variant["frequencies"][j];
                        line << "</td>\n";
                    }
                    for (int j = 0; j < 3; ++j) {
                        line << "<td";
                        if (mutated[j]) line << " id=\"hitP" + std::to_string(j) + "\" ";
                        line << ">";
                        if (static_cast<double>(variant["p-values"][j]) == 0)
                            line << "M";
                        else
                            line << static_cast<double>(variant["p-values"][j]);
                        line << "</td>\n";
                    }
                    for (int j = 0; j < 3; ++j) {
                        line << "<td";
                        if (mutated[j]) line << " id=\"hitP" + std::to_string(j) + "\" ";
                        line << ">";
                        line << variant["coverage"][j];
                        line << "</td>\n";
                    }
                    line << "<td>";
                    if (isKnown) line << strip(variant["known_drm"]);
                    line << "</td>\n";
                    if (first) {
                        out << prefix << line.str() << "</tr>" << std::endl;
                        first = false;
                    } else {
                        out << "<tr class=\"var\"><td></td><td></td><td></td>" << line.str()
                            << "</tr>" << std::endl;
                    }
                    line.str("");

                    out << R"(
                    <tr class="msa">
                    <td colspan=3 style="background-color: white"></td>
                    <td colspan=14 style="padding:0; margin:0">
                    <table style="padding:0; margin:0">
                    <col width="80px" />
                    <col width="80px" />
                    <col width="80px" />
                    <col width="80px" />
                    <col width="80px" />
                    <col width="80px" />
                    <tr style="padding:0">
                    <th style="padding:2px 0 0px 0">Pos</th>
                    <th style="padding:2px 0 0px 0">A</th>
                    <th style="padding:2px 0 0px 0">C</th>
                    <th style="padding:2px 0 0px 0">G</th>
                    <th style="padding:2px 0 0px 0">T</th>
                    <th style="padding:2px 0 0px 0">-</th>
                    </tr>
                    )";

                    for (auto& column : variant["msa_counts"]) {
                        int relPos = column["rel_pos"];
                        out << "<tr><td>" << relPos << "</td>" << std::endl;
                        for (int j = 0; j < 5; ++j) {
                            out << "<td style=\"";
                            if (relPos >= 0 && relPos < 3) {
                                if (j ==
                                    Data::NucleotideToTag(strip(variant["nucleotides"][relPos])[0]))
                                    out << "color:red;";
                            }
                            if (j == Data::NucleotideToTag(strip(column["wt"])[0]))
                                out << "font-weight:bold;";
                            out << "\">" << column[std::string(1, Data::TagToNucleotide(j))]
                                << "</td>" << std::endl;
                            /* code */
                        }
                        // out << "<td>" << column["A"] << "</td>" << std::endl;
                        // out << "<td>" << column["C"] << "</td>" << std::endl;
                        // out << "<td>" << column["G"] << "</td>" << std::endl;
                        // out << "<td>" << column["T"] << "</td>" << std::endl;
                        // out << "<td>" << column["-"] << "</td>" << std::endl;
                        out << "</tr>" << std::endl;
                    }
                    out << "</table></tr>" << std::endl;
                }
            }
        }
    }
    out << "</table>" << std::endl;
    out << "</body>" << std::endl;
    out << "</html>" << std::endl;
}

double ResistanceCaller::MaxFrequency(std::vector<VariantNucleotide> codon)
{
    double max = 0;
    for (int i = 0; i < 3; ++i) {
        const auto& f = codon[i].frequency;
        if (f != 1 && f > max) max = f;
    }
    return max == 0 ? 1 : max;
}

void ResistanceCaller::AddPosition(std::vector<VariantNucleotide>&& nucs)
{
    nucleotides_.emplace_back(std::forward<std::vector<VariantNucleotide>>(nucs));
}

std::string ResistanceCaller::CodonRef(int hxb2Position) const
{
    // if (hxb2Position % 3 != 0) throw std::runtime_error("Position is not a
    // multiple of three");
    if (hxb2Position + 2 >= end_) throw std::runtime_error("Position is out of window");
    return ref_.substr(hxb2Position, 3);
}

char ResistanceCaller::AminoacidRef(int hxb2Position) const
{
    // if (hxb2Position % 3 != 0) throw std::runtime_error("Position is not a
    // multiple of three");
    if (hxb2Position + 2 >= end_) throw std::runtime_error("Position is out of window");
    return codonToAmino_.at(ref_.substr(hxb2Position, 3));
}

inline std::string ResistanceCaller::CodonString(const std::vector<VariantNucleotide>& codon) const
{
    std::stringstream ss;
    ss << codon[0].nucleotide << codon[1].nucleotide << codon[2].nucleotide;
    return ss.str();
}

std::vector<std::vector<VariantNucleotide>> ResistanceCaller::CreateCodons(
    const int hxb2Position) const
{
    std::vector<std::vector<VariantNucleotide>> result;
    for (const auto& i : nucleotides_.at(hxb2Position - begin_ + 0))
        for (const auto& j : nucleotides_.at(hxb2Position - begin_ + 1))
            for (const auto& k : nucleotides_.at(hxb2Position - begin_ + 2))
                result.push_back({{i, j, k}});
    return result;
}

const std::unordered_map<int, std::string> ResistanceCaller::resistantCodon_ = {
    {2253 + 3 * 23, "PI S"},
    {2253 + 3 * 24, "PI + PI S"},
    {2253 + 3 * 30, "PI S"},
    {2253 + 3 * 32, "PI + PI S"},
    {2253 + 3 * 46, "PI + PI S"},
    {2253 + 3 * 47, "PI + PI S"},
    {2253 + 3 * 48, "PI S"},
    {2253 + 3 * 50, "PI + PI S"},
    {2253 + 3 * 53, "PI S"},
    {2253 + 3 * 54, "PI + PI S"},
    {2253 + 3 * 73, "PI S"},
    {2253 + 3 * 76, "PI + PI S"},
    {2253 + 3 * 82, "PI + PI S"},
    {2253 + 3 * 83, "PI S"},
    {2253 + 3 * 84, "PI + PI S"},
    {2253 + 3 * 85, "PI S"},
    {2253 + 3 * 88, "PI + PI S"},
    {2253 + 3 * 90, "PI + PI S"},
    {2550 + 3 * 100, "NNRTI + NRTI S"},
    {2550 + 3 * 101, "NNRTI + NRTI S"},
    {2550 + 3 * 103, "NNRTI + NRTI S"},
    {2550 + 3 * 106, "NNRTI + NRTI S"},
    {2550 + 3 * 115, "NRTI + NNRTI S"},
    {2550 + 3 * 116, "NNRTI S"},
    {2550 + 3 * 138, "NNRTI"},
    {2550 + 3 * 151, "NRTI + NNRTI S"},
    {2550 + 3 * 179, "NNRTI + NRTI S"},
    {2550 + 3 * 181, "NNRTI + NRTI S"},
    {2550 + 3 * 184, "NRTI + NNRTI S"},
    {2550 + 3 * 188, "NRTI S"},
    {2550 + 3 * 190, "NNRTI"},
    {2550 + 3 * 190, "NNRTI"},
    {2550 + 3 * 190, "NRTI S"},
    {2550 + 3 * 210, "NRTI + NNRTI S"},
    {2550 + 3 * 215, "NRTI + NNRTI S"},
    {2550 + 3 * 219, "NRTI + NNRTI S"},
    {2550 + 3 * 225, "NRTI S"},
    {2550 + 3 * 227, "NNRTI"},
    {2550 + 3 * 230, "NNRTI + NRTI S"},
    {2550 + 3 * 41, "NRTI + NNRTI S"},
    {2550 + 3 * 65, "NRTI + NNRTI S"},
    {2550 + 3 * 67, "NRTI + NNRTI S"},
    {2550 + 3 * 69, "NRTI + NNRTI S"},
    {2550 + 3 * 70, "NRTI + NNRTI S"},
    {2550 + 3 * 70, "NRTI"},
    {2550 + 3 * 74, "NRTI + NNRTI S"},
    {2550 + 3 * 75, "NNRTI S"},
    {2550 + 3 * 77, "NNRTI S"},
    {4230 + 3 * 138, "INI"},
    {4230 + 3 * 140, "INI"},
    {4230 + 3 * 143, "INI"},
    {4230 + 3 * 147, "INI"},
    {4230 + 3 * 148, "INI"},
    {4230 + 3 * 155, "INI"},
    {4230 + 3 * 66, "INI"},
    {4230 + 3 * 92, "INI"}};

const std::unordered_map<std::string, char> ResistanceCaller::codonToAmino_ = {
    {"ATT", 'I'}, {"ATC", 'I'}, {"ATA", 'I'}, {"CTT", 'L'}, {"CTC", 'L'}, {"CTA", 'L'},
    {"CTG", 'L'}, {"TTA", 'L'}, {"TTG", 'L'}, {"GTT", 'V'}, {"GTC", 'V'}, {"GTA", 'V'},
    {"GTG", 'V'}, {"TTT", 'F'}, {"TTC", 'F'}, {"ATG", 'M'}, {"TGT", 'C'}, {"TGC", 'C'},
    {"GCT", 'A'}, {"GCC", 'A'}, {"GCA", 'A'}, {"GCG", 'A'}, {"GGT", 'G'}, {"GGC", 'G'},
    {"GGA", 'G'}, {"GGG", 'G'}, {"CCT", 'P'}, {"CCC", 'P'}, {"CCA", 'P'}, {"CCG", 'P'},
    {"ACT", 'T'}, {"ACC", 'T'}, {"ACA", 'T'}, {"ACG", 'T'}, {"TCT", 'S'}, {"TCC", 'S'},
    {"TCA", 'S'}, {"TCG", 'S'}, {"AGT", 'S'}, {"AGC", 'S'}, {"TAT", 'Y'}, {"TAC", 'Y'},
    {"TGG", 'W'}, {"CAA", 'Q'}, {"CAG", 'Q'}, {"AAT", 'N'}, {"AAC", 'N'}, {"CAT", 'H'},
    {"CAC", 'H'}, {"GAA", 'E'}, {"GAG", 'E'}, {"GAT", 'D'}, {"GAC", 'D'}, {"AAA", 'K'},
    {"AAG", 'K'}, {"CGT", 'R'}, {"CGC", 'R'}, {"CGA", 'R'}, {"CGG", 'R'}, {"AGA", 'R'},
    {"AGG", 'R'}, {"TAA", 'X'}, {"TAG", 'X'}, {"TGA", 'X'}};

const std::string ResistanceCaller::ref_ =
    "TGGAAGGGCTAATTCACTCCCAACGAAGACAAGATATCCTTGATCTGTGGATCTACCACACACAAGGCTACTTC"
    "CCTGATTAGCAG"
    "AACTACACACCAGGGCCAGGGATCAGATATCCACTGACCTTTGGATGGTGCTACAAGCTAGTACCAGTTGAGCC"
    "AGAGAAGTTAGA"
    "AGAAGCCAACAAAGGAGAGAACACCAGCTTGTTACACCCTGTGAGCCTGCATGGAATGGATGACCCGGAGAGAG"
    "AAGTGTTAGAGT"
    "GGAGGTTTGACAGCCGCCTAGCATTTCATCACATGGCCCGAGAGCTGCATCCGGAGTACTTCAAGAACTGCTGA"
    "CATCGAGCTTGC"
    "TACAAGGGACTTTCCGCTGGGGACTTTCCAGGGAGGCGTGGCCTGGGCGGGACTGGGGAGTGGCGAGCCCTCAG"
    "ATCCTGCATATA"
    "AGCAGCTGCTTTTTGCCTGTACTGGGTCTCTCTGGTTAGACCAGATCTGAGCCTGGGAGCTCTCTGGCTAACTA"
    "GGGAACCCACTG"
    "CTTAAGCCTCAATAAAGCTTGCCTTGAGTGCTTCAAGTAGTGTGTGCCCGTCTGTTGTGTGACTCTGGTAACTA"
    "GAGATCCCTCAG"
    "ACCCTTTTAGTCAGTGTGGAAAATCTCTAGCAGTGGCGCCCGAACAGGGACCTGAAAGCGAAAGGGAAACCAGA"
    "GGAGCTCTCTCG"
    "ACGCAGGACTCGGCTTGCTGAAGCGCGCACGGCAAGAGGCGAGGGGCGGCGACTGGTGAGTACGCCAAAAATTT"
    "TGACTAGCGGAG"
    "GCTAGAAGGAGAGAGATGGGTGCGAGAGCGTCAGTATTAAGCGGGGGAGAATTAGATCGATGGGAAAAAATTCG"
    "GTTAAGGCCAGG"
    "GGGAAAGAAAAAATATAAATTAAAACATATAGTATGGGCAAGCAGGGAGCTAGAACGATTCGCAGTTAATCCTG"
    "GCCTGTTAGAAA"
    "CATCAGAAGGCTGTAGACAAATACTGGGACAGCTACAACCATCCCTTCAGACAGGATCAGAAGAACTTAGATCA"
    "TTATATAATACA"
    "GTAGCAACCCTCTATTGTGTGCATCAAAGGATAGAGATAAAAGACACCAAGGAAGCTTTAGACAAGATAGAGGA"
    "AGAGCAAAACAA"
    "AAGTAAGAAAAAAGCACAGCAAGCAGCAGCTGACACAGGACACAGCAATCAGGTCAGCCAAAATTACCCTATAG"
    "TGCAGAACATCC"
    "AGGGGCAAATGGTACATCAGGCCATATCACCTAGAACTTTAAATGCATGGGTAAAAGTAGTAGAAGAGAAGGCT"
    "TTCAGCCCAGAA"
    "GTGATACCCATGTTTTCAGCATTATCAGAAGGAGCCACCCCACAAGATTTAAACACCATGCTAAACACAGTGGG"
    "GGGACATCAAGC"
    "AGCCATGCAAATGTTAAAAGAGACCATCAATGAGGAAGCTGCAGAATGGGATAGAGTGCATCCAGTGCATGCAG"
    "GGCCTATTGCAC"
    "CAGGCCAGATGAGAGAACCAAGGGGAAGTGACATAGCAGGAACTACTAGTACCCTTCAGGAACAAATAGGATGG"
    "ATGACAAATAAT"
    "CCACCTATCCCAGTAGGAGAAATTTATAAAAGATGGATAATCCTGGGATTAAATAAAATAGTAAGAATGTATAG"
    "CCCTACCAGCAT"
    "TCTGGACATAAGACAAGGACCAAAGGAACCCTTTAGAGACTATGTAGACCGGTTCTATAAAACTCTAAGAGCCG"
    "AGCAAGCTTCAC"
    "AGGAGGTAAAAAATTGGATGACAGAAACCTTGTTGGTCCAAAATGCGAACCCAGATTGTAAGACTATTTTAAAA"
    "GCATTGGGACCA"
    "GCGGCTACACTAGAAGAAATGATGACAGCATGTCAGGGAGTAGGAGGACCCGGCCATAAGGCAAGAGTTTTGGC"
    "TGAAGCAATGAG"
    "CCAAGTAACAAATTCAGCTACCATAATGATGCAGAGAGGCAATTTTAGGAACCAAAGAAAGATTGTTAAGTGTT"
    "TCAATTGTGGCA"
    "AAGAAGGGCACACAGCCAGAAATTGCAGGGCCCCTAGGAAAAAGGGCTGTTGGAAATGTGGAAAGGAAGGACAC"
    "CAAATGAAAGAT"
    "TGTACTGAGAGACAGGCTAATTTTTTAGGGAAGATCTGGCCTTCCTACAAGGGAAGGCCAGGGAATTTTCTTCA"
    "GAGCAGACCAGA"
    "GCCAACAGCCCCACCAGAAGAGAGCTTCAGGTCTGGGGTAGAGACAACAACTCCCCCTCAGAAGCAGGAGCCGA"
    "TAGACAAGGAAC"
    "TGTATCCTTTAACTTCCCTCAGGTCACTCTTTGGCAACGACCCCTCGTCACAATAAAGATAGGGGGGCAACTAA"
    "AGGAAGCTCTAT"
    "TAGATACAGGAGCAGATGATACAGTATTAGAAGAAATGAGTTTGCCAGGAAGATGGAAACCAAAAATGATAGGG"
    "GGAATTGGAGGT"
    "TTTATCAAAGTAAGACAGTATGATCAGATACTCATAGAAATCTGTGGACATAAAGCTATAGGTACAGTATTAGT"
    "AGGACCTACACC"
    "TGTCAACATAATTGGAAGAAATCTGTTGACTCAGATTGGTTGCACTTTAAATTTTCCCATTAGCCCTATTGAGA"
    "CTGTACCAGTAA"
    "AATTAAAGCCAGGAATGGATGGCCCAAAAGTTAAACAATGGCCATTGACAGAAGAAAAAATAAAAGCATTAGTA"
    "GAAATTTGTACA"
    "GAGATGGAAAAGGAAGGGAAAATTTCAAAAATTGGGCCTGAAAATCCATACAATACTCCAGTATTTGCCATAAA"
    "GAAAAAAGACAG"
    "TACTAAATGGAGAAAATTAGTAGATTTCAGAGAACTTAATAAGAGAACTCAAGACTTCTGGGAAGTTCAATTAG"
    "GAATACCACATC"
    "CCGCAGGGTTAAAAAAGAAAAAATCAGTAACAGTACTGGATGTGGGTGATGCATATTTTTCAGTTCCCTTAGAT"
    "GAAGACTTCAGG"
    "AAGTATACTGCATTTACCATACCTAGTATAAACAATGAGACACCAGGGATTAGATATCAGTACAATGTGCTTCC"
    "ACAGGGATGGAA"
    "AGGATCACCAGCAATATTCCAAAGTAGCATGACAAAAATCTTAGAGCCTTTTAGAAAACAAAATCCAGACATAG"
    "TTATCTATCAAT"
    "ACATGGATGATTTGTATGTAGGATCTGACTTAGAAATAGGGCAGCATAGAACAAAAATAGAGGAGCTGAGACAA"
    "CATCTGTTGAGG"
    "TGGGGACTTACCACACCAGACAAAAAACATCAGAAAGAACCTCCATTCCTTTGGATGGGTTATGAACTCCATCC"
    "TGATAAATGGAC"
    "AGTACAGCCTATAGTGCTGCCAGAAAAAGACAGCTGGACTGTCAATGACATACAGAAGTTAGTGGGGAAATTGA"
    "ATTGGGCAAGTC"
    "AGATTTACCCAGGGATTAAAGTAAGGCAATTATGTAAACTCCTTAGAGGAACCAAAGCACTAACAGAAGTAATA"
    "CCACTAACAGAA"
    "GAAGCAGAGCTAGAACTGGCAGAAAACAGAGAGATTCTAAAAGAACCAGTACATGGAGTGTATTATGACCCATC"
    "AAAAGACTTAAT"
    "AGCAGAAATACAGAAGCAGGGGCAAGGCCAATGGACATATCAAATTTATCAAGAGCCATTTAAAAATCTGAAAA"
    "CAGGAAAATATG"
    "CAAGAATGAGGGGTGCCCACACTAATGATGTAAAACAATTAACAGAGGCAGTGCAAAAAATAACCACAGAAAGC"
    "ATAGTAATATGG"
    "GGAAAGACTCCTAAATTTAAACTGCCCATACAAAAGGAAACATGGGAAACATGGTGGACAGAGTATTGGCAAGC"
    "CACCTGGATTCC"
    "TGAGTGGGAGTTTGTTAATACCCCTCCCTTAGTGAAATTATGGTACCAGTTAGAGAAAGAACCCATAGTAGGAG"
    "CAGAAACCTTCT"
    "ATGTAGATGGGGCAGCTAACAGGGAGACTAAATTAGGAAAAGCAGGATATGTTACTAATAGAGGAAGACAAAAA"
    "GTTGTCACCCTA"
    "ACTGACACAACAAATCAGAAGACTGAGTTACAAGCAATTTATCTAGCTTTGCAGGATTCGGGATTAGAAGTAAA"
    "CATAGTAACAGA"
    "CTCACAATATGCATTAGGAATCATTCAAGCACAACCAGATCAAAGTGAATCAGAGTTAGTCAATCAAATAATAG"
    "AGCAGTTAATAA"
    "AAAAGGAAAAGGTCTATCTGGCATGGGTACCAGCACACAAAGGAATTGGAGGAAATGAACAAGTAGATAAATTA"
    "GTCAGTGCTGGA"
    "ATCAGGAAAGTACTATTTTTAGATGGAATAGATAAGGCCCAAGATGAACATGAGAAATATCACAGTAATTGGAG"
    "AGCAATGGCTAG"
    "TGATTTTAACCTGCCACCTGTAGTAGCAAAAGAAATAGTAGCCAGCTGTGATAAATGTCAGCTAAAAGGAGAAG"
    "CCATGCATGGAC"
    "AAGTAGACTGTAGTCCAGGAATATGGCAACTAGATTGTACACATTTAGAAGGAAAAGTTATCCTGGTAGCAGTT"
    "CATGTAGCCAGT"
    "GGATATATAGAAGCAGAAGTTATTCCAGCAGAAACAGGGCAGGAAACAGCATATTTTCTTTTAAAATTAGCAGG"
    "AAGATGGCCAGT"
    "AAAAACAATACATACTGACAATGGCAGCAATTTCACCGGTGCTACGGTTAGGGCCGCCTGTTGGTGGGCGGGAA"
    "TCAAGCAGGAAT"
    "TTGGAATTCCCTACAATCCCCAAAGTCAAGGAGTAGTAGAATCTATGAATAAAGAATTAAAGAAAATTATAGGA"
    "CAGGTAAGAGAT"
    "CAGGCTGAACATCTTAAGACAGCAGTACAAATGGCAGTATTCATCCACAATTTTAAAAGAAAAGGGGGGATTGG"
    "GGGGTACAGTGC"
    "AGGGGAAAGAATAGTAGACATAATAGCAACAGACATACAAACTAAAGAATTACAAAAACAAATTACAAAAATTC"
    "AAAATTTTCGGG"
    "TTTATTACAGGGACAGCAGAAATCCACTTTGGAAAGGACCAGCAAAGCTCCTCTGGAAAGGTGAAGGGGCAGTA"
    "GTAATACAAGAT"
    "AATAGTGACATAAAAGTAGTGCCAAGAAGAAAAGCAAAGATCATTAGGGATTATGGAAAACAGATGGCAGGTGA"
    "TGATTGTGTGGC"
    "AAGTAGACAGGATGAGGATTAGAACATGGAAAAGTTTAGTAAAACACCATATGTATGTTTCAGGGAAAGCTAGG"
    "GGATGGTTTTAT"
    "AGACATCACTATGAAAGCCCTCATCCAAGAATAAGTTCAGAAGTACACATCCCACTAGGGGATGCTAGATTGGT"
    "AATAACAACATA"
    "TTGGGGTCTGCATACAGGAGAAAGAGACTGGCATTTGGGTCAGGGAGTCTCCATAGAATGGAGGAAAAAGAGAT"
    "ATAGCACACAAG"
    "TAGACCCTGAACTAGCAGACCAACTAATTCATCTGTATTACTTTGACTGTTTTTCAGACTCTGCTATAAGAAAG"
    "GCCTTATTAGGA"
    "CACATAGTTAGCCCTAGGTGTGAATATCAAGCAGGACATAACAAGGTAGGATCTCTACAATACTTGGCACTAGC"
    "AGCATTAATAAC"
    "ACCAAAAAAGATAAAGCCACCTTTGCCTAGTGTTACGAAACTGACAGAGGATAGATGGAACAAGCCCCAGAAGA"
    "CCAAGGGCCACA"
    "GAGGGAGCCACACAATGAATGGACACTAGAGCTTTTAGAGGAGCTTAAGAATGAAGCTGTTAGACATTTTCCTA"
    "GGATTTGGCTCC"
    "ATGGCTTAGGGCAACATATCTATGAAACTTATGGGGATACTTGGGCAGGAGTGGAAGCCATAATAAGAATTCTG"
    "CAACAACTGCTG"
    "TTTATCCATTTTCAGAATTGGGTGTCGACATAGCAGAATAGGCGTTACTCGACAGAGGAGAGCAAGAAATGGAG"
    "CCAGTAGATCCT"
    "AGACTAGAGCCCTGGAAGCATCCAGGAAGTCAGCCTAAAACTGCTTGTACCAATTGCTATTGTAAAAAGTGTTG"
    "CTTTCATTGCCA"
    "AGTTTGTTTCATAACAAAAGCCTTAGGCATCTCCTATGGCAGGAAGAAGCGGAGACAGCGACGAAGAGCTCATC"
    "AGAACAGTCAGA"
    "CTCATCAAGCTTCTCTATCAAAGCAGTAAGTAGTACATGTAACGCAACCTATACCAATAGTAGCAATAGTAGCA"
    "TTAGTAGTAGCA"
    "ATAATAATAGCAATAGTTGTGTGGTCCATAGTAATCATAGAATATAGGAAAATATTAAGACAAAGAAAAATAGA"
    "CAGGTTAATTGA"
    "TAGACTAATAGAAAGAGCAGAAGACAGTGGCAATGAGAGTGAAGGAGAAATATCAGCACTTGTGGAGATGGGGG"
    "TGGAGATGGGGC"
    "ACCATGCTCCTTGGGATGTTGATGATCTGTAGTGCTACAGAAAAATTGTGGGTCACAGTCTATTATGGGGTACC"
    "TGTGTGGAAGGA"
    "AGCAACCACCACTCTATTTTGTGCATCAGATGCTAAAGCATATGATACAGAGGTACATAATGTTTGGGCCACAC"
    "ATGCCTGTGTAC"
    "CCACAGACCCCAACCCACAAGAAGTAGTATTGGTAAATGTGACAGAAAATTTTAACATGTGGAAAAATGACATG"
    "GTAGAACAGATG"
    "CATGAGGATATAATCAGTTTATGGGATCAAAGCCTAAAGCCATGTGTAAAATTAACCCCACTCTGTGTTAGTTT"
    "AAAGTGCACTGA"
    "TTTGAAGAATGATACTAATACCAATAGTAGTAGCGGGAGAATGATAATGGAGAAAGGAGAGATAAAAAACTGCT"
    "CTTTCAATATCA"
    "GCACAAGCATAAGAGGTAAGGTGCAGAAAGAATATGCATTTTTTTATAAACTTGATATAATACCAATAGATAAT"
    "GATACTACCAGC"
    "TATAAGTTGACAAGTTGTAACACCTCAGTCATTACACAGGCCTGTCCAAAGGTATCCTTTGAGCCAATTCCCAT"
    "ACATTATTGTGC"
    "CCCGGCTGGTTTTGCGATTCTAAAATGTAATAATAAGACGTTCAATGGAACAGGACCATGTACAAATGTCAGCA"
    "CAGTACAATGTA"
    "CACATGGAATTAGGCCAGTAGTATCAACTCAACTGCTGTTAAATGGCAGTCTAGCAGAAGAAGAGGTAGTAATT"
    "AGATCTGTCAAT"
    "TTCACGGACAATGCTAAAACCATAATAGTACAGCTGAACACATCTGTAGAAATTAATTGTACAAGACCCAACAA"
    "CAATACAAGAAA"
    "AAGAATCCGTATCCAGAGAGGACCAGGGAGAGCATTTGTTACAATAGGAAAAATAGGAAATATGAGACAAGCAC"
    "ATTGTAACATTA"
    "GTAGAGCAAAATGGAATAACACTTTAAAACAGATAGCTAGCAAATTAAGAGAACAATTTGGAAATAATAAAACA"
    "ATAATCTTTAAG"
    "CAATCCTCAGGAGGGGACCCAGAAATTGTAACGCACAGTTTTAATTGTGGAGGGGAATTTTTCTACTGTAATTC"
    "AACACAACTGTT"
    "TAATAGTACTTGGTTTAATAGTACTTGGAGTACTGAAGGGTCAAATAACACTGAAGGAAGTGACACAATCACCC"
    "TCCCATGCAGAA"
    "TAAAACAAATTATAAACATGTGGCAGAAAGTAGGAAAAGCAATGTATGCCCCTCCCATCAGTGGACAAATTAGA"
    "TGTTCATCAAAT"
    "ATTACAGGGCTGCTATTAACAAGAGATGGTGGTAATAGCAACAATGAGTCCGAGATCTTCAGACCTGGAGGAGG"
    "AGATATGAGGGA"
    "CAATTGGAGAAGTGAATTATATAAATATAAAGTAGTAAAAATTGAACCATTAGGAGTAGCACCCACCAAGGCAA"
    "AGAGAAGAGTGG"
    "TGCAGAGAGAAAAAAGAGCAGTGGGAATAGGAGCTTTGTTCCTTGGGTTCTTGGGAGCAGCAGGAAGCACTATG"
    "GGCGCAGCCTCA"
    "ATGACGCTGACGGTACAGGCCAGACAATTATTGTCTGGTATAGTGCAGCAGCAGAACAATTTGCTGAGGGCTAT"
    "TGAGGCGCAACA"
    "GCATCTGTTGCAACTCACAGTCTGGGGCATCAAGCAGCTCCAGGCAAGAATCCTGGCTGTGGAAAGATACCTAA"
    "AGGATCAACAGC"
    "TCCTGGGGATTTGGGGTTGCTCTGGAAAACTCATTTGCACCACTGCTGTGCCTTGGAATGCTAGTTGGAGTAAT"
    "AAATCTCTGGAA"
    "CAGATTTGGAATCACACGACCTGGATGGAGTGGGACAGAGAAATTAACAATTACACAAGCTTAATACACTCCTT"
    "AATTGAAGAATC"
    "GCAAAACCAGCAAGAAAAGAATGAACAAGAATTATTGGAATTAGATAAATGGGCAAGTTTGTGGAATTGGTTTA"
    "ACATAACAAATT"
    "GGCTGTGGTATATAAAATTATTCATAATGATAGTAGGAGGCTTGGTAGGTTTAAGAATAGTTTTTGCTGTACTT"
    "TCTATAGTGAAT"
    "AGAGTTAGGCAGGGATATTCACCATTATCGTTTCAGACCCACCTCCCAACCCCGAGGGGACCCGACAGGCCCGA"
    "AGGAATAGAAGA"
    "AGAAGGTGGAGAGAGAGACAGAGACAGATCCATTCGATTAGTGAACGGATCCTTGGCACTTATCTGGGACGATC"
    "TGCGGAGCCTGT"
    "GCCTCTTCAGCTACCACCGCTTGAGAGACTTACTCTTGATTGTAACGAGGATTGTGGAACTTCTGGGACGCAGG"
    "GGGTGGGAAGCC"
    "CTCAAATATTGGTGGAATCTCCTACAGTATTGGAGTCAGGAACTAAAGAATAGTGCTGTTAGCTTGCTCAATGC"
    "CACAGCCATAGC"
    "AGTAGCTGAGGGGACAGATAGGGTTATAGAAGTAGTACAAGGAGCTTGTAGAGCTATTCGCCACATACCTAGAA"
    "GAATAAGACAGG"
    "GCTTGGAAAGGATTTTGCTATAAGATGGGTGGCAAGTGGTCAAAAAGTAGTGTGATTGGATGGCCTACTGTAAG"
    "GGAAAGAATGAG"
    "ACGAGCTGAGCCAGCAGCAGATAGGGTGGGAGCAGCATCTCGAGACCTGGAAAAACATGGAGCAATCACAAGTA"
    "GCAATACAGCAG"
    "CTACCAATGCTGCTTGTGCCTGGCTAGAAGCACAAGAGGAGGAGGAGGTGGGTTTTCCAGTCACACCTCAGGTA"
    "CCTTTAAGACCA"
    "ATGACTTACAAGGCAGCTGTAGATCTTAGCCACTTTTTAAAAGAAAAGGGGGGACTGGAAGGGCTAATTCACTC"
    "CCAAAGAAGACA"
    "AGATATCCTTGATCTGTGGATCTACCACACACAAGGCTACTTCCCTGATTAGCAGAACTACACACCAGGGCCAG"
    "GGGTCAGATATC"
    "CACTGACCTTTGGATGGTGCTACAAGCTAGTACCAGTTGAGCCAGATAAGATAGAAGAGGCCAATAAAGGAGAG"
    "AACACCAGCTTG"
    "TTACACCCTGTGAGCCTGCATGGGATGGATGACCCGGAGAGAGAAGTGTTAGAGTGGAGGTTTGACAGCCGCCT"
    "AGCATTTCATCA"
    "CGTGGCCCGAGAGCTGCATCCGGAGTACTTCAAGAACTGCTGACATCGAGCTTGCTACAAGGGACTTTCCGCTG"
    "GGGACTTTCCAG"
    "GGAGGCGTGGCCTGGGCGGGACTGGGGAGTGGCGAGCCCTCAGATCCTGCATATAAGCAGCTGCTTTTTGCCTG"
    "TACTGGGTCTCT"
    "CTGGTTAGACCAGATCTGAGCCTGGGAGCTCTCTGGCTAACTAGGGAACCCACTGCTTAAGCCTCAATAAAGCT"
    "TGCCTTGAGTGC"
    "TTCAAGTAGTGTGTGCCCGTCTGTTGTGTGACTCTGGTAACTAGAGATCCCTCAGACCCTTTTAGTCAGTGTGG"
    "AAAATCTCTAGC"
    "A";
}
}  // ::PacBio::Juliet