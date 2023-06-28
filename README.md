
[![build_linux](https://github.com/bankofcanada/DataEcon/actions/workflows/build_linux.yml/badge.svg)](https://github.com/bankofcanada/DataEcon/actions/workflows/build_linux.yml)
[![build_windows](https://github.com/bankofcanada/DataEcon/actions/workflows/build_windows.yml/badge.svg)](https://github.com/bankofcanada/DataEcon/actions/workflows/build_windows.yml)
[![build_macos](https://github.com/bankofcanada/DataEcon/actions/workflows/build_macos.yml/badge.svg)](https://github.com/bankofcanada/DataEcon/actions/workflows/build_macos.yml)
[![codecov](https://codecov.io/gh/bankofcanada/DataEcon/branch/main/graph/badge.svg?token=5BE01J5G6W)](https://codecov.io/gh/bankofcanada/DataEcon)

# DataEcon

This repository contains the C code for a shared library that is used under the
hood of [TimeSeriesEcon.jl](https://github.com/bankofcanada/TimeSeriesEcon.jl)
for reading and writing time series data in Julia.

From Julia simply install the
[TimeSeriesEcon](https://github.com/bankofcanada/TimeSeriesEcon.jl) package.

> **NOTE** that `DataEcon` is not yet part of an official release of
> `TimeSeriesEcon`. You can give it a try from the `dataecon` branch like this:
> ```julia
> ] add TimeSeriesEcon#dataecon 
> ```

If not using Julia, the rest of this file should help you to get started. 

## Getting started

Use `make` to build the library and run the tests.

For example,
```bash
bash$ make lib
```
will build the library `bin/libdaec.so`.

Similarly,
```bash
bash$ make test
```
will build a test application `bin/test` and run it.

Alternatively, you can simply download precompiled binaries for your machine from [DataEcon_jll](https://github.com/JuliaBinaryWrappers/DataEcon_jll.jl/releases).

## Using the library

To build your own executable, compile it with `#include "src/daec.h"`, and link it against the `bin/libdaec.so` library.  

For example:
```C
// example.c
#include "daec.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    de_file de;
    int rc;

    rc = de_open("example.daec", &de);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to open the file.\n");
        return EXIT_FAILURE;
    }

    char message[] = "Hello World";
    rc = de_store_scalar(de, 0, "message", type_string, freq_none,
                         sizeof message + 1, message, NULL);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to write the message\n");
        de_close(de);
        return EXIT_FAILURE;
    }

    obj_id_t id;
    rc = de_find_fullpath(de, "/message", &id);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to find the message.\n");
        de_close(de);
        return EXIT_FAILURE;
    }

    scalar_t scalar;
    rc = de_load_scalar(de, id, &scalar);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to read the message.\n");
        de_close(de);
        return EXIT_FAILURE;
    }

    printf("%s\n", (char *)scalar.value);

    de_close(de);
    return EXIT_SUCCESS;
}

```

```bash
bash$ gcc -I ./src -L bin -Wl,-rpath,$(pwd)/bin -ldaec example.c -o example
bash$ ./example
Hello World
```

