// Copyright (c) 2014-2015, Pacific Biosciences of California, Inc.
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

#include <thread>

#include <pacbio/Version.h>
#include <pacbio/data/PlainOption.h>
#include <boost/algorithm/string.hpp>

#include <pacbio/cleric/ClericSettings.h>

namespace PacBio {
namespace Cleric {
namespace OptionNames {
using PlainOption = Data::PlainOption;
// clang-format off
const PlainOption Output{
    "output",
    { "output", "o"},
    "Output BAM File Name",
    "Output file name for generated bam file [Default: Input file prefix + _cleric.bam].",
    CLI::Option::StringType("")
};
// clang-format on
}  // namespace OptionNames

ClericSettings::ClericSettings(const PacBio::CLI::Results &options)
    : InputFiles(options.PositionalArguments())
{
    if (!options[OptionNames::Output].empty())
        OutputPrefix = std::forward<std::string>(options[OptionNames::Output]);
}
PacBio::CLI::Interface ClericSettings::CreateCLI()
{
    using Option = PacBio::CLI::Option;
    using Task = PacBio::CLI::ToolContract::Task;

    PacBio::CLI::Interface i{
        "cleric", "Cleric, a BAM alignment reference converter",
        PacBio::MinorseqVersion() + " (commit " + PacBio::MinorseqGitSha1() + ")"};

    i.AddHelpOption();     // use built-in help output
    i.AddVersionOption();  // use built-in version output

    // clang-format off
    i.AddPositionalArguments({
        {"source", "Source BAM or DataSet XML file.", "FILE"}
    });

    i.AddOptions(
    {
        OptionNames::Output
    });

    const std::string id = "uny.tasks.cleric";
    Task tcTask(id);

    tcTask.InputFileTypes({
        {
            "alignment_set",
            "AlignmentSet",
            "Alignment DataSet or aligned .bam file",
            "PacBio.DataSet.AlignmentSet"
        }
    });

    CLI::ToolContract::Config tcConfig(tcTask);
    i.EnableToolContract(tcConfig);

    // clang-format on

    return i;
}
}
}  // ::PacBio::CCS