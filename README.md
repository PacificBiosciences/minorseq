<p align="center">
  <img src="doc/img/minorseq.png" alt="minorseq logos" width="400px"/>
</p>
<h1 align="center">MinorSeq</h1>
<p align="center">Minor Variant Calling and Phasing Tools</p>

***
## Availability
The latest pre-release, developers-only linux binaries can be installed via [bioconda](https://bioconda.github.io/):

    conda install minorseq

These binaries are not ISO compliant.
For research only.
Not for use in diagnostics procedures.

Official support is only provided for official and stable [SMRT Analysis builds](http://www.pacb.com/products-and-services/analytical-software/)
provided by PacBio.

Unofficial support for binary pre-releases is provided via github issues,
not via mail to developers.

## Quick Tools Overview

### [End-to-end workflow](doc/INTRODUCTION.md)

Overview how to run your sample.

### [Minor variant caller](doc/JULIET.md)

`juliet` identifies minor variants from aligned ccs reads.

### [Reduce alignment](doc/FUSE.md)

`fuse` reduces an alignment into its closest representative sequence.

### [Swap BAM reference](doc/CLERIC.md)

`cleric` swaps the reference of an alignment by transitive alignment.

### [Minor variant pipeline](doc/JULIETFLOW.md)

`julietflow` automatizes the minor variant pipeline.

### [Mix Data _In-Silico_](doc/MIXDATA.md)

`mixdata` helps to mix clonal strains _in-silico_ for benchmarking studies.

## Disclaimer

THIS WEBSITE AND CONTENT AND ALL SITE-RELATED SERVICES, INCLUDING ANY DATA, ARE PROVIDED "AS IS," WITH ALL FAULTS, WITH NO REPRESENTATIONS OR WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTIES OF MERCHANTABILITY, SATISFACTORY QUALITY, NON-INFRINGEMENT OR FITNESS FOR A PARTICULAR PURPOSE. YOU ASSUME TOTAL RESPONSIBILITY AND RISK FOR YOUR USE OF THIS SITE, ALL SITE-RELATED SERVICES, AND ANY THIRD PARTY WEBSITES OR APPLICATIONS. NO ORAL OR WRITTEN INFORMATION OR ADVICE SHALL CREATE A WARRANTY OF ANY KIND. ANY REFERENCES TO SPECIFIC PRODUCTS OR SERVICES ON THE WEBSITES DO NOT CONSTITUTE OR IMPLY A RECOMMENDATION OR ENDORSEMENT BY PACIFIC BIOSCIENCES.