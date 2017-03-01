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

#include <pacbio/juliet/JsonToHtml.h>
#include <string>

namespace PacBio {
namespace Juliet {

/// Generate HTML output of variant amino acids

void JsonToHtml::HTML(std::ostream& out, const JSON::Json& j, const std::string referenceName,
                      bool onlyKnownDRMs, bool details)
{
#if 1
    // Count number of haplotypes
    int numHaplotypes = 0;
    for (const auto& gene : j["genes"]) {
        for (auto& variantPosition : gene["variant_positions"]) {
            for (auto& variant_amino_acid : variantPosition["variant_amino_acids"]) {
                for (auto& variant_codons : variant_amino_acid["variant_codons"]) {
                    numHaplotypes = variant_codons["haplotype_hit"].size();
                    break;
                }
            }
        }
    }

    auto strip = [](const std::string& input) {
        std::string s = input;
        s.erase(std::remove(s.begin(), s.end(), '\"'), s.end());
        return s;
    };
    out << "<!-- Juliet Minor Variant Summary by Dr. Armin Toepfer (Pacific Biosciences) -->"
        << std::endl
        << "<html>" << std::endl
        << "<head>" << std::endl
        << R"(
            <script src="http://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js"></script>
            <script type="text/javascript">
            $(document).ready(function() {
                $(".var").bind( "click", function( event ) {
                    $(this).next().slideToggle(0);
            });
            });
            </script>)"
        << std::endl
        << "<style>" << std::endl
        << R"(
            body { font-family: helvetica-light }
            table { border-collapse: collapse; margin-bottom: 20px; }
            tr:nth-child(1) { background-color: #3d3d3d; color: white; }
            tr:nth-child(3) th { padding: 5px 5px 5px 5px; text-align: center; border-bottom: 1px solid #2d2d2d; }
            tr:nth-child(2) th:nth-child(2) { border-left: 1px dashed black; }
            tr:nth-child(3) th:nth-child(3) { border-right: 1px dashed black; })";
    if (numHaplotypes)
        out << R"(
            tr:nth-child(2) th:nth-child(2) { border-right: 1px dashed black; }
            tr:nth-child(3) th:nth-child(8) { border-right: 1px dashed black; })";
    out << R"(
            td { padding: 15px 5px 15px 5px; text-align: center; border-bottom: 1px solid white; }
            table td:nth-child(1) { background-color:#ddd; border-right: 1px solid #eee; }
            table td:nth-child(2) { background-color:#eee; border-right: 1px solid #ddd; }
            table td:nth-child(3) { background-color:#fff; border-right: 1px solid #ddd; font-weight: bold;}
            table td:nth-child(4) { background-color:#eee; border-right: 1px dashed #ccc;  }
            table td:nth-child(5) { background-color: #ddd; border-right: 1px dashed #bbb; }
            table td:nth-child(6) { background-color: #ccc; border-right: 1px dashed #aaa; }
            table td:nth-child(7) { background-color: #bbb;}
            table td:nth-child(8) { background-color: #aaa; color: white}
            table td:nth-child(8) { border-right:1px solid white;}
            table td:nth-child(n+9) { border-left:1px dotted white;}
            table td:nth-child(n+9) { background-color: #4a4a4a;}
            tr:not(.msa):hover td { background-color: white; }
            tr:not(.msa):hover td:nth-child(8) { color: purple; }
            .msa table tr:hover td { background-color: gray; color:white; }
            .top table { background-color:white; border:0; }
            .top table td { background-color:white; border:0; border-bottom: 1px solid gray; font-weight: normal}
            .top table tr { border:0; }
            .top table th { border:0; }
            .msa { display:none; }
            )"
        << std::endl
        << "</style>" << std::endl
        << "</head>" << std::endl
        << R"(<body>
            <details style="margin-bottom: 20px">
            <summary>Legend</summary>
            <p>Every table represents a gene.<br/>
            Each row stands for a mutated amino acid. Positions are relative to the current gene.<br/>
            Positions with no or synonymous mutation are not being shown.<br/>
            All coordinates are in reference space.<br/>
            The mutated nucleotide is highlighted in the codon.<br/>
            Frequency is per codon.<br/>
            Coverage includes deletions.<br/>
            Known drug-resistance mutations positions are annotated in the last column,<br/>
            whereas 'S' stands for surveillance. Annotations from the <a href="https://hivdb.stanford.edu" target="_new">Stanford DB</a>.<br/>
            <br/>
            Clicking on a row unfolds the counts of the multiple sequence alignment of the<br/>
            codon position and up to +-3 surrounding positions.<br/>
            Red colored are nucleotides of the codon and in bold the wild type.<br/>
            <br/>
            Deletions and insertions are being ignored in this version.<br/>
            <br/>
            This software is for research only and has not been clinically validated!</p>
            </details>)"
        << std::endl;

    if (j.find("genes") == j.cend() || j["genes"].is_null()) return;
    for (const auto& gene : j["genes"]) {
        out << "<table class=\"top\">" << std::endl
            << R"(
                <col width="40px"/>
                <col width="40px"/>
                <col width="40px"/>
                <col width="40px"/>
                <col width="40px"/>
                <col width="60px"/>
                <col width="60px"/>
                <col width="180px"/>)";
        for (int hap = 0; hap < numHaplotypes; ++hap) {
            out << R"(<col width="40"/>)";
        }
        out << R"(<tr>
                <th colspan=")"
            << (8 + numHaplotypes) << R"(">)" << strip(gene["name"]) << R"(</th>
                </tr>
                <tr>
                <th colspan="3">)";
        if (referenceName.empty()) out << "unknown";
        if (referenceName.size() > 11)
            out << referenceName.substr(0, 11) << "...";
        else
            out << referenceName;
        out << R"(</th>
                <th colspan="5">Sample</th>)";
        if (numHaplotypes > 0) {
            out << R"(<th colspan=")" << numHaplotypes << R"(">Haplotypes %</th>)";
        }
        out << R"(
                </tr>
                <tr>
                <th>Codon</th>
                <th>AA</th>
                <th>Pos</th>
                <th>AA</th>
                <th>Codon</th>
                <th>Frequency</th>
                <th>Coverage</th>
                <th>DRM</th>)";
        for (int hap = 0; hap < numHaplotypes; ++hap) {
            out << R"(<th>)"
                << std::round(1000 * static_cast<double>(j["haplotypes"][hap]["frequency"])) / 10;
            out << "</th>";
        }
        out << R"(</tr>)" << std::endl;

        for (auto& variantPosition : gene["variant_positions"]) {
            std::stringstream line;
            const std::string refCodon = strip(variantPosition["ref_codon"]);
            line << "<tr class=\"var\">\n"
                 << "<td>" << refCodon[0] << " " << refCodon[1] << " " << refCodon[2] << "</td>\n"
                 << "<td>" << strip(variantPosition["ref_amino_acid"]) << "</td>\n"
                 << "<td>" << variantPosition["ref_position"] << "</td>";
            std::string prefix = line.str();
            line.str("");
            bool first = true;
            for (auto& variant_amino_acid : variantPosition["variant_amino_acids"]) {
                for (auto& variant_codons : variant_amino_acid["variant_codons"]) {
                    bool mutated[]{refCodon[0] != strip(variant_codons["codon"])[0],
                                   refCodon[1] != strip(variant_codons["codon"])[1],
                                   refCodon[2] != strip(variant_codons["codon"])[2]};
                    line << "<td>" << strip(variant_amino_acid["amino_acid"]) << "</td>";
                    line << "<td>";
                    for (int j = 0; j < 3; ++j) {
                        if (mutated[j]) line << "<b style=\"color:#B50A36; font-weight:normal\">";
                        line << strip(variant_codons["codon"])[j] << " ";
                        if (mutated[j]) line << "</b>";
                    }

                    double fOrig = variant_codons["frequency"];
                    double fTmp;
                    int exp = 0;
                    do {
                        fTmp = fOrig * std::pow(10, ++exp);
                    } while (static_cast<int>(fTmp) < 10);
                    fOrig = static_cast<int>(fOrig * std::pow(10, exp));
                    fOrig /= std::pow(10, exp);
                    line << "<td>" << fOrig << "</td>";
                    if (first) {
                        out << prefix << line.str();
                        out << "<td>" << variantPosition["coverage"] << "</td>";
                        first = false;
                    } else {
                        out << "<tr class=\"var\"><td></td><td></td><td></td>" << line.str()
                            << "<td></td>";
                    }
                    out << "<td>" << strip(variant_codons["known_drm"]) << "</td>";
                    static std::vector<std::string> colors = {"#b50937", "#b22450", "#b2385e",
                                                              "#b24f6e", "#b2647c", "#b27487",
                                                              "#b28391", "#b2909b", "#b29ea5"};
                    int col = 0;
                    for (auto& hit : variant_codons["haplotype_hit"]) {
                        if (hit)
                            out << "<td style=\"background-color:" << colors.at(col % colors.size())
                                << "\"></td>";
                        else
                            out << "<td></td>";
                        ++col;
                    }
                    out << "</tr>" << std::endl;
                    line.str("");

                    out << R"(
                        <tr class="msa">
                        <td colspan=3 style="background-color: white"></td>
                        <td colspan=14 style="padding:0; margin:0">
                        <table style="padding:0; margin:0">
                        <col width="50px" />
                        <col width="67px" />
                        <col width="67px" />
                        <col width="67px" />
                        <col width="67px" />
                        <col width="67px" />
                        <col width="67px" />
                        <tr style="padding:0">
                        <th style="padding:2px 0 0px 0">Pos</th>
                        <th style="padding:2px 0 0px 0">A</th>
                        <th style="padding:2px 0 0px 0">C</th>
                        <th style="padding:2px 0 0px 0">G</th>
                        <th style="padding:2px 0 0px 0">T</th>
                        <th style="padding:2px 0 0px 0">-</th>
                        <th style="padding:2px 0 0px 0">N</th>
                        </tr>
                        )";

                    for (auto& column : variantPosition["msa"]) {
                        int relPos = column["rel_pos"];
                        out << "<tr><td>" << relPos << "</td>" << std::endl;
                        for (int j = 0; j < 6; ++j) {
                            out << "<td style=\"";
                            if (relPos >= 0 && relPos < 3 &&
                                j ==
                                    Data::NucleotideToTag(strip(variant_codons["codon"])[relPos])) {
                                out << "color:#B50A36;";
                            }
                            if (j == Data::NucleotideToTag(strip(column["wt"])[0]))
                                out << "font-weight:bold;";
                            out << "\">" << column[std::string(1, Data::TagToNucleotide(j))]
                                << "</td>" << std::endl;
                        }
                        out << "</tr>" << std::endl;
                    }
                    out << "</table></tr>" << std::endl;
                }
            }
        }
    }
    out << "</table>" << std::endl << "</body>" << std::endl << "</html>" << std::endl;
#endif
}
}
}  //::PacBio::Juliet