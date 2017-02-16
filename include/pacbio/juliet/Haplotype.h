#pragma once

#include <pacbio/util/Termcolor.h>

namespace PacBio {
namespace Juliet {
struct Haplotype
{
    std::vector<std::string> Names;
    std::vector<std::string> Codons;
    double SoftCollapses = 0;
    bool NoGaps = true;

    double Size() const { return Names.size() + SoftCollapses; }

    std::string ConcatCodons() const
    {
        return std::accumulate(Codons.begin(), Codons.end(), std::string(""));
    }

    void SetCodons(std::vector<std::string>&& codons)
    {
        Codons = std::forward<std::vector<std::string>>(codons);
        for (const auto& c : Codons) {
            if (c.find('-') != std::string::npos) {
                NoGaps = false;
                break;
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& stream, const Haplotype& h)
    {
        stream << h.Size() << "\t";
        for (const auto& c : h.Codons) {
            if (c != "ATG" && c != "AAA" && c != "TAT" && c != "GGA" && c != "ACC")
                stream << termcolor::white;
            if (c == "TTG") stream << termcolor::red;
            if (c == "AGA") stream << termcolor::green;
            if (c == "TGT") stream << termcolor::blue;
            if (c == "GCA") stream << termcolor::blue;
            if (c == "TAC") stream << termcolor::yellow;
            stream << " " << c << termcolor::reset;
        }
        return stream;
    }
};
}
}