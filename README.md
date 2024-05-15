# hunter_search

This is a parallel program in C++ for targeted Hunter searches. It is part of the PhD thesis **[ref]**.


## Usage

### Building

We build the progam using `make`. If `gmp` is not installed, one should do this first (see Dependencies below). In order to build the program, we use the following command from the root directory.

```
make all DEG=<degree>
```

Here, `<degree>` should be substituted for the desired degree of the Hunter search we want to perform. One can also compile a version of the progam which collects more statistics about the Hunter search during runtime (such as the total number of polynomials enumerated). However, this does slow down running times by approximately 30% in practice. For this, we build using the `all-stats` option.

```
make all-stats DEG=<degree>
```

### Dependencies

To run (and in the first case compile) the program, the following libraries need to be installed.

* [`gmp`](https://gmplib.org/) version `>= 6.1.1`. In [share](share), a version of `gmp` is included. It can be installed in this folder by running the following commands.  

        cd share  
        tar -xf gmp-6.1.1.tar.xz  
        make install-all

* [`mpi`](https://www.open-mpi.org/). The program has been tested using `Open MPI` versions `>= 1.10.3`.

### Running the program

After building the program, an executable can be found in `bin` (`bin/enum` or `bin/enum-stats`). The executable can be run in parallel using the `mpirun` command. For an example, we refer to [example](example).

Several options (some of which are mandatory for the progarm to run) can be passed when running the program.

+ -disc (mandatory): The discriminant bound for the Hunter search.
+ -divisors (mandatory)>: A comma-separated list of positive integers which the coefficients of the defining polynomials should be divisible by. This list should be exactly as long as the degree of the Hunter search.
+ -tree (mandatory): A file containing the discriminant tree in a specific text format (as produced by the Mathematica script [misc/disc_tree.nb](misc/disc_tree.nb)).
+ -locked (mandatory): A positive integer $1 \le k <= \text{degree}-1$, which represents the number of coefficients which are fixed by the root node and are then distributed to different worker nodes.
+ -input (optional): A comma-separated list of text files containing congruences for the resulting defining polynomials.
+ -output (optional): The path of the output file (default: `output.txt`).
+ -coeffs (optional): A comma-separated list of $1\le k\le \text{degree}$ integers. If this option is set to $A_1,\dots,A_k$ then the first $k$ coefficients (in the ordering used) of the resulting defining polynomials will be equal to $A_1,\dots,A_k$.
+ -filter (optional): A comma-separated list of prime numbers which are used to further filter the discriminant up to squares modulo these primes. Using this option can improve search times.
+ -limit (optional): A positive interger $k$ or $-1$. If this option is set to $k > 1$ then the program will stop after processing $k$ work units (as distributed by the root node). If this option is set to $-1$ or not set at all then the Hunter serach will continue until all work units are processed. This option can be used to do some rough time estimates for longer searches.


## Contents



## References

**[ref]**
