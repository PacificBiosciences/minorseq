## How to run your sample 101

### Step 1
A simple bioconda installation:

```sh
conda install minorseq
```

--------
### Step 2
Create CCS2 reads from your sequel chip

> Juliet currently uses PacBio CCS reads as input. The use of CCS rich QVs allows sensitive minor variant calling.

```
ccs --richQVs m54000_170101_050702_3545456.subreadset.xml yourdata.ccs.bam
```

--------
### Step 3
Filter CCS2 reads as described here: [JULIETFLOW.md#filtering](JULIETFLOW.md#filtering)

> To ensure a uniform noise profile, we filter to 99% predicted
>  accuracy. Barcode demultiplexing might be done.

--------
### Step 4
Download the reference sequence of interest as `ref.fasta`

> Juliet currently calls amino acid variants to a given refrence
>  sequence so they might be easily related to known variants.

--------
### Step 5
Create a target-config for your gene as described here: [JULIET.md#target-configuration](JULIET.md#target-configuration)

> The target-config specifies Open Reading and how specific amino
>  acids should be labeled in output results (ie Disease Resistant
>  Mutation variants)


--------
### Step 6
Run *julietflow*

> The calling sequence is very simple taking sequencing reads, the
>  reference, and the reference annotation config.

```
julietflow -i yourdata.filtered.ccs.bam -r ref.fasta -c targetconfig.json
```

--------
### Step 7
Interpret results in `yourdata.json` or `yourdata.html`

> 'yourdata.html' is easily viewed in a web browser and reflects the
>  underlying results stored in 'yourdata.json'