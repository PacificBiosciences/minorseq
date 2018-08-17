<h1 align="center">
    fuse - Reduce alignment into its representative sequence
</h1>

<p align="center">
  <img src="img/fuse.png" alt="Logo of Fuse" width="100px"/>
</p>

## Install
Install the minorseq suite using bioconda, more info [here](../README.md).
One of the binaries is called `fuse`.

## Input data
*Fuse* operates on aligned records in the BAM format.
BAM files have to PacBio-compliant, meaning, cigar `M` is forbidden.

## Scope
Current scope of *Fuse* is creation of a high-quality consensus sequence.
Fuse includes in-frame insertions with a certain distance to each other.
Major deletions are being removed.

## Output
*Fuse* provides a FASTA file per input. Output file is provided by the second
argument.

## Example
Simple example:
```
fuse m530526.align.bam m530526.fasta
```

Output: `m530526.fasta`