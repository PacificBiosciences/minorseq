<h1 align="center">
    mixdata - Generate <i>In-Silico</i> Mixtures
</h1>

## Install
Install the minorseq suite using bioconda, more info [here](../README.md).
The script is called `mixdata`.

## About
*Mixdata* operates on BAM files.
It mixes given BAM files at a given ratio and coverage.
The first file is taken as the major clone, all following files are mixed in
as minors with the provided percentage.

## Options
The *mixdata* script uses env variables for parametrization:

|Parameter|Description|Default|
|-|-|-|
|`COVERAGE`|The coverage of the final mixed BAM file.|3000|
|`PERCENTAGE`|The percentage of each minor clone.|1|
|`OUTPUT_PREFIX`|The output prefix of the generated file.|mix|

## Example
### Defaults
Mix three BAM files with 98/1/1% at 3000x:
```
$ ls
clone_1.bam clone_2.bam clone_3.bam
$ mixdata clone_*.bam
$ samtools view mix.bam | wc -l
3000
```

Equivalent call:
```
$ mixdata clone_1.bam clone_2.bam clone_3.bam
```

### Configuration
Mix five BAM files with
 - 10% minors
 - `clone_2.bam` as major with 60%
 - 6000x coverage
 - file name `insilico.bam`
```
$ ls
clone_1.bam clone_2.bam clone_3.bam clone_4.bam clone_5.bam
$ COVERAGE=6000 PERCENTAGE=10 OUTPUT_PREFIX=insilico mixdata clone_{2,1,3,4,5}.bam
$ samtools view insilico.bam | wc -l
6000
```

## Dependencies
Only dependency is [samtools](https://github.com/samtools/samtools), which is
part of the SMRT-bundle that is officially being provided by PacBio.