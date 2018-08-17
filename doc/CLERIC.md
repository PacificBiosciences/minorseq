<h1 align="center">
    Cleric - Swap BAM alignment reference
</h1>

<p align="center">
  <img src="img/cleric.png" alt="Logo of Cleric" width="200px"/>
</p>

## Install
Install the minorseq suite using bioconda, more info [here](../README.md).
One of the binaries is called `cleric`.

## Input data
*Cleric* operates on aligned records in the BAM format, the original reference
and the target reference as FASTA.
BAM file have to PacBio-compliant, meaning, cigar `M` is forbidden.
Two sequences have to be provided, either in individual files or combined in one.
The header of the original reference must match the reference name in the BAM.

## Scope
Current scope of *Cleric* is converting a given alignment to a different
reference. This is done by aligning the original and target reference sequences.
A transitive alignment is used to generate the new alignment.

## Output
*Cleric* provides a BAM file with the file named as provided via the last argument.

## Example
Simple example:
```
cleric m530526.align.bam reference.fasta new_ref.fasta cleric_output.bam
```

Or:
```
cat reference.fasta new_ref.fasta > combined.fasta
cleric m530526.align.bam combined.fasta cleric_output.bam
```

## FAQ
### Cleric does not finish.
Runtime is linear in the number of reads provided. The alignment step runs a
Needleman-Wunsch; with NxM runtime. Please do not provide references with
lengths of human chromosomes, but concentrate on your actual amplicon target.