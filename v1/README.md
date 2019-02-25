# About WISE v1.0 (wise)

*By Jean-François Grailet (last updated: February 25, 2019)*

## Compilation

You will need gcc and g++ on your Linux distribution to compile `WISE` (`wise`). To compile it, set *Release/* as your working directory and execute the command:

```sh
make
```

If you need to recompile `WISE` after some editing, type the following commands:

```sh
make clean
make
```

## Deployement on PlanetLab testbed

If you intent to use `WISE` from the PlanetLab testbed, here is some advice.

* Do not bother with compiling `WISE` on PlanetLab nodes and rather compile it on your own computer. Then, you can upload the executable file (found in *Release/*) on a PlanetLab node and uses it as soon as you connect to it.

* Of course, your executable should be compiled with an environement similar to that of the PlanetLab nodes. The oldest OS you should find on a PLC (PlanetLab Central) node is usually Fedora 8 (at the time this file was written). A safe (but slow) method to compile `WISE` for Fedora 8 and onwards is to run Fedora 8 as a virtual machine, put the sources on it, compile `WISE` and retrieve the executable file. Note that most if not all PLC nodes are 32-bit machines.

* PLE (PlanetLab Europe) nodes uses 64-bit versions of much more recent releases of Fedora (e.g., Fedora 24). To run `WISE` as compiled for PLC nodes one PLE nodes, if the PLE node cannot run `WISE` yet, check what libraries it currently has and copy the 32-bit libraries from the PLC nodes on it (usually, these libraries are just missing). Thanks to this trick, you will be able to run `WISE` as compiled for PLC nodes. Make sure, however, to double-check what 32-bit libraries are already available on the PLE nodes to not overwrite existing libraries.

## Usage

`WISE` v1.0 will describe in details its options, flags and how you can use it by running the line:

```sh
./wise -h
```

## Configuration files

In order to simplify the parameters of `WISE` and only allow the editing of the most important parameters in command-line, specific probing parameters are only editable with specific configuration files. You can find an example of such configuration file in *Release/* (with the default configuration of `WISE`).

## Remarks

* Most machines forbid the user to open sockets to send probes, which prevents `WISE` from doing anything. To overcome this, run `WISE` as a super user (for example, with `sudo`).

* Most of the actual code of `WISE` is found in *src/algo/*. *src/prober/* and *src/common/* provides libraries to handle (ICMP, UDP, TCP) probes, IPv4 addresses, etc. If you wish to build a completely different application using ICMP/UDP/TCP probing, you can take the full code of ``WISE`` and just remove the *src/algo/* folder.

* If you intend to remove or add files to the source code, you have to edit the subdir.*mk* files in *Release/src/* accordingly. You should also edit *makefile* and sources.*mk* in *Release/* whenever you create a new (sub-)directory in *src/*.

## Changes history

* **February 25, 2019:** release of `WISE` v1.0.

## Disclaimer

`WISE` was written by Jean-François Grailet, currently Ph. D. student at the University of Liège (Belgium) in the Research Unit in Networking (RUN). Some parts of `WISE` re-uses code from `SAGE`. For more details on `SAGE`, check its public repository:

https://github.com/JefGrailet/sage

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/

Feel free to send me an e-mail if your encounter problems while compiling or running `WISE`. I am also inclined to answer questions regarding the algorithms used in `WISE` and to discuss its application in other research projects.
