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

In order to run this program, one needs to have installed [`gmp`](https://gmplib.org/) version `>= 6.1.1`. In [share](share), a version of `gmp` is included. It can be installed in this folder by running the following commands.

```
cd share
tar -xf gmp-6.1.1.tar.xz
make install-all
```

### Running the program



## Contents



## References

**[ref]**
