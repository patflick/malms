# malms: Malleable Sorting

This code implements a malleable thread scheduler and its application to
parallel sorting. We implemented a malleable parallel sorting algorithm and
compare its performance under difference parallel loads against the GNU parallel
sort (MCSTL) and TBB sort.

The algorithms and experimental results are described in the following peer-reviewed publication:

> Flick, Patrick, Peter Sanders, and Jochen Speck. "Malleable sorting." *Parallel & Distributed Processing (IPDPS), 2013 IEEE 27th International Symposium on.* IEEE, 2013.  [![doi.org/10.1109/IPDPS.2013.90](https://img.shields.io/badge/doi-10.1109%2FIPDPS.2013.90-blue.svg)](https://doi.org/10.1109/IPDPS.2013.90)


### Code overview:

The code-base is structured as follows:

- `malms/` contains the implementation of the malleable scheduler and malleable
  parallel sorting algorithm
- `test` implements some test cases for the malleable parallel sorting algorithm
- `utils` contains small programs we use to block cores, generate input, etc
- `timing` contains the executables and scripts used for generating the
  experimental results.

### Building

#### Build-Requirements

The build depends on `boost-thread` and Thread Building Blocks `tbb`.
In Ubuntu, these dependencies can be installed using:

```sh
sudo apt-get install libboost-thread-dev libtbb-dev
```

#### Make

To build, simply run `make` in the repositories root directory.
