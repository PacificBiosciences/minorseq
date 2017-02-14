<p align="center">
  <img src="doc/img/juliet.png" alt="juliet logo" width="200px"/>
</p>
<h1 align="center">MinorSeq</h1>
<p align="center">Minor Variant Calling and Phasing Tools</p>

***
## Documentation

 - [Getting Started](doc/INSTALL.md)
 - Projects
   - [Minor Variant Calling `juliet`](doc/JULIET.md)
   - [Reduce Alignment `fuse`](doc/FUSE.md)
   - [Swap Alignment Reference `cleric`](doc/CLERIC.md)
   - [Minor Variant Pipeline `julietflow`](doc/JULIETFLOW.md)
 - [Developer environment](doc/DEVELOPER.md)
 - [PacBio open source license](LICENSE)

## Quick Tools Overview

### [Minor variant caller](doc/JULIET.md)

`juliet` identifies minor variants from aligned ccs reads.

### [Reduce alignment](doc/FUSE.md)

`fuse` reduces an alignment into its closest representative sequence.

### [Swap BAM reference](doc/CLERIC.md)

`cleric` swaps the reference of an alignment by transitive alignment.

### [Minor variant pipeline](doc/JULIETFLOW.md)

`julietflow` automatizes the minor variant pipeline.

## Help

Official support is only provided for official and stable
[SMRT Analysis builds](http://www.pacb.com/products-and-services/analytical-software/)
provided by PacBio and not for source builds.
Unofficial support for source builds is provided via github issues.