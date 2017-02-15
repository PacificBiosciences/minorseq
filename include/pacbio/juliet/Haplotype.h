#pragma once

struct Haplotype
{
    std::vector<std::string> Names;
    std::vector<std::string> Codons;
    bool NoGaps = true;

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
        stream << h.Names.size();
        for (const auto& c : h.Codons)
            stream << " " << c;
        return stream;
    }
};