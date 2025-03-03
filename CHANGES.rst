=======
Changes
=======

v1.2 (2021-12-08)
-----------------

* :issue:`208`: Fix ``phase --merge-reads``. This option has never worked correctly and just led to
  ``whatshap phase`` taking a very long time and in some cases even crashing. With the fix, the
  option should work as intended, but we have not evaluated how much it improves phasing results.
* :issue:`337`: Add ``--skip-missing-contigs`` option to ``whatshap haplotag``
* :pr:`335`: Add option ``--ignore-sample-name`` to ``whatshap compare`` (thanks to Pontus Höjer)
* :issue:`342`: Fix ``whatshap compare`` crashing on VCFs with genotypes with an unknown allele
  (where ``GT`` is ``1|.`` or similar).
* :issue:`343`: ``whatshap stats`` now reads the chromosome lengths (for N50 computation) from
  the VCF header, no need to use ``--chr-lengths``.

v1.1 (2021-04-08)
-----------------

* :issue:`223`: Fix ``haplotag --ignore-linked-reads`` not working
* :issue:`241`: Fix some ``polyphase`` problems.
* :issue:`249`: Fix crash in the ``haplotag`` command on reading a VCF with the
  ``PS`` tag set to ``.``.
* :issue:`251`: Allow ``haplotag`` to correctly write to standard output.
* :issue:`207`: Allow multiple ``--chromosome`` arguments to ``stats``.
* The file created with ``--output-read-list`` was not correctly tab-separated.
* :issue:`248`: Remove ``phase --full-genotyping`` option. Instead, use ``whatshap genotype``
  followed by ``whatshap phase``.
* :issue:`289`: Fix parsing of GVCFs (with dots in the ALT column)
* :pr:`265`: ``polyphase`` can now work in parallel

v1.0 (2020-06-24)
-----------------

WhatsHap has not seen a release in over a year although development has continued. To make up for
it, we decided to leave `ZeroVer <https://0ver.org/>`_ behind and set the version number to 1.0.

* WhatsHap has gained initial support for phasing polyploid samples! While this feature may not be
  quite ready for production use, we encourage you to test it by using the ``whatshap polyphase``
  subcommand and to report any issues you find back to us. See also the pre-print at
  <https://doi.org/10.1101/2020.02.04.933523> for details.
* :issue:`51`: Reading and writing VCF files is now significantly faster because we switched
  to a different library for that task (``pysam.VariantFile``).
* The switch to ``pysam.VariantFile`` also makes WhatsHap stricter in which VCF files it accepts.
  We have tried to give sensible error messages in these cases, but please report any remaining
  issues.
* ``.bcf`` files can now be read and written.
* :issue:`110`: ``.vcf.gz`` output files are now compressed with bgzip so that they can be
  indexed with tabix.
* Providing an indexed reference FASTA is now mandatory (with ``-r`` or ``--reference``). It
  is possible to bypass this by using ``--no-reference``, but that will disable realignment and
  therefore give worse phasing results on error-prone reads (PacBio, Nanopore).
* :issue:`187`: Implemented a ``--regions`` option for the ``haplotag`` subcommand.
* Implemented a ``--discard-unknown-reads`` option for the ``split`` subcommand. Reads that are in
  the input reads file (BAM/FASTQ), but are not listed in the haplotag file will be
  discarded (by default, they are part of the "untagged" output).
* Fixed :issue:`215`. ``split`` subcommand can now process ``.bam`` files lacking the
  ``sequence`` field for some/all reads.
* The minimum required Python version for WhatsHap is now 3.6.

v0.18 (2019-02-15)
------------------
* Add option ``--plot-sum-of-blocksizes`` to ``whatshap compare``.
* Fix in ``whatshap stats``: sometimes returned wrong N50 values if the end
  position of the last block of a chromosome was larger than the starting position
  of the first block of the next chromosome.
* :issue:`173`: The ``haplotag`` command should now be able to properly write
  CRAM files.
* :issue:`177`: Option ``--ignore-read-groups`` did not work when phased blocks
  (VCF) were provided as input.
* :issue:`122`: Add ``--ignore-read-groups`` and ``--samples`` options to ``haplotag``.
* Integration of the HapChat algorithm as an alternative MEC solver, available
  through ``whatshap phase --algorithm=hapchat``. Contributed by the HapChat
  team, see https://doi.org/10.1186/s12859-018-2253-8.
* This is the last release of WhatsHap to support Python 3.4.

v0.17 (2018-07-20)
------------------
* :issue:`140`: Haplotagging now works when chromosomes are missing in the VCF.
* Added option ``--merge-reads``, which is helpful for high coverage data.
* When phasing pedigrees, ensure that haplotypes are ordered as
  paternal_allele|maternal_allele in the output VCF. This seems to be a common
  convention and also used by 1000G.
* Test cases now use pytest instead of nose (which is discontinued).

v0.16 (2018-05-22)
------------------

* :issue:`167`: Fix the ``haplotag`` command. It would tag reads incorrectly.
* :issue:`154`: Use barcode information in BX tags when running ``haplotag``
  on 10x Genomics linked read data.
* :issue:`153`: Allow combination of ``--ped`` and ``--samples`` to only work
  on a subset of samples in a pedigree. Added ``--use-ped-samples`` to only
  phase samples mentioned in PED file (while ignoring other samples in input VCF).

v0.15 (2018-04-07)
------------------

* New subcommand ``genotype`` for haplotype-aware genotyping 
  (see https://doi.org/10.1101/293944 for details on the method).
* Support CRAM files in addition to BAM.
* :issue:`133`:
  No longer create BAM/CRAM index if it does not exist. This is safer when running multiple
  WhatsHap instances in parallel. From now on, you need to create the index yourself
  (for example with ``samtools index``) before running WhatsHap.
* :issue:`152`: Reads marked as “duplicate” in the input BAM/CRAM file are now ignored.
* :issue:`157`: Adapt to changed interface in Pysam 0.14.
* :issue:`158`: Handle read groups with missing sample (SM) tag correctly.

v0.14.1 (2017-07-07)
--------------------

* Fix compilation problem by distinguishing gcc and clang.

v0.14 (2017-07-06)
------------------

* Added ``--full-genotyping`` to (re-)genotype the given variants based on the reads
* Added option ``whatshap compare --switch-error-bed`` to write BED file with switch
  error positions
* Added ``whatshap compare --plot-blocksizes`` to plot histogroms of block sizes
* Added option ``--longest-block-tsv`` to output position-wise stats on longest joint
  haplotype block
* Added option ``whatshap compare --tsv-multiway`` to write results of multi-way
  comparison to tab-separated file
* Added option --chromosome to whatshap stats
* ``whatshap compare`` can now compute the block-wise Hamming distance
* ``whatshap stats`` can now compute an N50 for the phased blocks
* Fixed compilation issues on OS X (clang)
* Detect unsorted VCFs and chromosome name mismatches between BAM and VCF
* Fix crash when whatshap compare encounteres unphased VCFs
* Expanded documentation.

v0.13 (2016-10-27)
------------------

* Use ``PS`` tag instead of ``HP`` tag by default to store phasing information.
  This applies to the ``phase`` and ``hapcut2vcf`` subcommands. ``PS`` is also
  used by other tools and standard according to the VCF specification.
* Incorporated genotype likelihoods into our phasing framework. On request
  (by using option ``--distrust-genotypes``), genotypes can now be changed at a cost
  corresponding to their input genotype likelihoods. The changed genotypes are
  written to the output VCF. The behavior of ``--distrust-genotypes`` can be
  fine-tuned by the added options ``--include-homozygous``, ``--default-gq``,
  ``--gl-regularizer``, and ``--changed-genotype-list``.
* Correctly handle cases when processing VCFs with two or more disjoint
  families.

v0.12 (2016-07-01)
------------------

* Speed up allele detection
* Add an ``unphase`` subcommand which removes all phasing from a VCF file
  (``HP`` and ``PS`` tags, pipe notation).
* Add option ``--tag=`` to the ``phase`` subcommand, which allows to choose
  whether ReadBackedPhasing-compatible ``HP`` tags or standard ``PS`` tags are
  used to describe phasing in the output VCF.
* Manage versions with `versioneer <https://github.com/warner/python-versioneer>`_.
  This means that ``whatshap --version`` and the program version in the VCF header
  will include the Git commit hash, such as ``whatshap 0.11+50.g1b7af7a``.
* Add subcommand "haplotag" to tag reads in a BAM file with their haplotype.
* Fix a bug where re-alignment around variants at the very end of a chromosome
  would lead to an AssertionError.

v0.11 (2016-06-09)
------------------

* When phasing a pedigree, blocks that are not connected by reads but
  can be phased based on genotypes will be connected per default. This
  behavior can be turned off using option ``--no-genetic-haplotyping``.
* Implemented allele detection through re-alignment: To detect which allele of a
  variant is seen in a read, the query is aligned to the two haplotypes at that
  position. This results in better quality phasing, especially for
  low-quality reads (PacBio). Enabled if ``--reference`` is provided. Current
  limitation: No score for the allele is computed.
* As a side-effect of the new allele detection, we can now also phase
  insertions, deletions, MNPs and "complex" variants.
* Added option ``--chromosome`` to only work on specifed chromosomes.
* Use constant recombination rate per default, allows to use ``--ped``
  without using ``--genmap``.
* ``whatshap`` has become a command with subcommands. From now on, you need
  to run ``whatshap phase`` to phase VCFs.
* Add a ``stats`` subcommand that prints statistics about phased VCFs.

v0.10 (2016-04-27)
------------------

* Use ``--ped`` to phase pedigrees with the PedMEC algorithm
* Phase all samples in a multi-sample VCF
* Drop support for Python 3.2 - we require at least Python 3.3 now

v0.9 (2016-01-05)
-----------------

* This is the first release available via PyPI (and that can therefore be
  installed via ``pip install whatshap``)

January 2016
------------

* Trio phasing implemented in a branch

September 2015
--------------

* pWhatsHap implemented (in a branch)

April 2015
----------

* Create haplotype-specific BAM files

February 2015
-------------

* Smart read selection

January 2015
------------

* Ability to read multiple BAM files and merge them on the fly

December 2014
-------------

* Logo
* Unit tests

November 2014
-------------

* Cython wrapper for C++ code done
* Ability to write a phased VCF (using HP tags).

June 2014
---------

* Repository for WhatsHap refactoring created

April 2014
----------

* The WhatsHap algorithm is introduced at RECOMB
