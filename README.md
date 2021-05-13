# Towards Declarative Self-adapting Buffer Management (Simulation code)

## Requirements

 * [OTcl](https://sourceforge.net/projects/otcl-tclcl/files/OTcl/1.14/)
 * [TCLCL](https://sourceforge.net/projects/otcl-tclcl/files/TclCL/1.20/)
 * [Tcl/Tk](https://www.tcl.tk/software/tcltk/download.html)
 * [CMake](https://cmake.org/)
 * [Boost](https://www.boost.org/)
 * [nlohmann json](https://github.com/nlohmann/json)
 * [click](https://click.palletsprojects.com/)
 * [numpy](https://www.numpy.org/)
 * [matplotlib](https://matplotlib.org/)
 * [sqlite](https://www.sqlite.org/index.html)

## Preparation steps

 * compile the learning

    ```bash
    cd self-adapting-simulator
    mkdir cmake-build-release
    cd cmake-build-release
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    cd ..
    ```

 * compile ns2

    ```bash
    cd ns2
    source setenv.sh
    mkdir cmake-build-release
    cd cmake-build-release
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    cd ..
    ```

 * prepare environment

    ```bash
    cd self-adapting-simulator/ns2
    source setenv.sh
    ```

##  Running simulations

The main script to run simulations is called `run_codel.py` which shows an extensive list of parameters if run with `--help`. Once the simulation has finished, the plots from the paper can be obtained by running `plot_reward` function from `plotting.py`. The raw data is available in `testik.db` sqlite3 database.

