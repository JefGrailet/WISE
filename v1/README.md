# About WISE v1.1 (wise)

*By Jean-François Grailet (last updated: January 7, 2020)*

## About the code

Since it needs to be compatible with old environments (e.g. Fedora 8, see remarks on deployment below), `WISE` is written in an _old-fashioned_ C++, i.e., it doesn't take advantage of the changes brought by C++11 and onwards. As already said elsewhere, it is also designed to be a 32-bit application for the same reason. This said, after its many campaigns run from the PlanetLab testbed towards all kinds of target networks, it is safe to assume `WISE` is unlikely to crash or mismanage memory. It has been, on top of that, been extensively tested with `valgrind` on a local network.

## About the sub-folders "Legacy" and "Tool"

In late October 2019, the source code of `WISE` has been thoroughly reviewed before being used as a basis for a future topology discovery tool. There were two motivations for this:
* to remove old classes and other old pieces of code inherited from `ExploreNET` (`ExploreNET` being the basis of `TreeNET` and the first version of `SAGE`), 
* to reduce the amount of dynamic memory allocations overall.

The refreshed `WISE` is available in the *Tool/* sub-directory. However, for curious programmers, the *old* code of `WISE` is still available in the *Legacy/* directory. This *old* version is, of course, still working and can still be deployed for measurement campaigns if needed.

## Compilation

You will need gcc and g++ on your Linux distribution to compile `WISE` (`wise`). To compile it, set *Tool/Release/* (or *Legacy/Release/*) as your working directory and execute the command:

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

* PLE (PlanetLab Europe) nodes uses 64-bit versions of much more recent releases of Fedora (e.g., Fedora 24). To run `WISE` as compiled for PLC nodes on PLE nodes, if the PLE node cannot run `WISE` yet, check what libraries it currently has and copy the 32-bit libraries from the PLC nodes on it (usually, these libraries are just missing). Thanks to this trick, you will be able to run `WISE` as compiled for PLC nodes. Make sure, however, to double-check what 32-bit libraries are already available on the PLE nodes to not overwrite existing libraries.

## Usage

`WISE` v1.1 will describe in details its options, flags and how you can use it by running the line:

```sh
./wise -h
```

## Configuration files

In order to simplify the parameters of `WISE` and only allow the editing of the most important parameters in command-line, specific probing parameters are only editable with specific configuration files. You can find an example of such configuration file in *Release/* (with the default configuration of `WISE`).

## Remarks

* Most machines forbid the user to open sockets to send probes, which prevents `WISE` from doing anything. To overcome this, run `WISE` as a super user (for example, with `sudo`).

* Most of the actual code of `WISE` is found in *src/algo/* (in either *Tool/* or *Legacy/* sub-directories). *src/prober/* and *src/common/* provides libraries to handle (ICMP, UDP, TCP) probes, IPv4 addresses, etc. If you wish to build a completely different application using ICMP/UDP/TCP probing, you can take the full code of ``WISE`` and just remove the *src/algo/* folder.

* If you intend to remove or add files to the source code, you have to edit the subdir.*mk* files in *Release/src/* accordingly. You should also edit *makefile* and sources.*mk* in *Release/* whenever you create a new (sub-)directory in *src/*.

## Changes history

* **February 25, 2019:** release of `WISE` v1.0.

* **September 19, 2019:** release of `WISE` v1.1. This new version provides an updated and improved subnet post-processing, as well as one additional step after subnet inference designed to infer *neighborhoods*, network locations bordered by subnets that are located at at most one hop from each other which can consist of a single router or a mesh of network-layer/datalink-layer devices. In addition to inferring neighborhoods, `WISE` v1.1 also locates neighborhoods w.r.t. each other by using some additional data collected with partial (Paris) `traceroute` measurements. Neighborhoods will be later used to infer more from a target domain.

* **October 14, 2019:** update of `WISE` v1.1. In addition to fixing a minor issue that can occur during neighborhood inference (rare occurrences of cycles which can induce bad _peer_ discovery), this update extends the fingerprinting of `WISE` (used for alias resolution) by adding the inferred initial TTL value of the ICMP "_Time exceeded_" replies, as used by Vanaubel et al. in "_Network Fingerprinting: TTL-Based Router Signatures_" (IMC 2013). This TTL value wasn't used in previous tools which the alias resolution module come from due to the impossibility to get this value for all alias candidates. In the context of `WISE`, this restriction no longer exists.

* **November 8, 2019:** update of `WISE` v1.1, now available in a re-hauled version that can be found in the *Tool/* sub-directory (the previous code being available in *Legacy/*) which takes slightly better advantage of features of C++ while removing old, unused classes inherited from other topology discovery tools. This update also fixes minor issues with specific scenarii of subnet inference, such as re-evaluating a pivot interface as a contra-pivot interface (additional checks have been added to prevent an erroneous diagnostic in situations where TTL distances vary considerably).

* **November 19, 2019:** new update of `WISE` v1.1 (both *Legacy/* and *Tool/*) to further refine neighborhood inference. This update fixes a rare but annoying inference issue which we will denote as *self-peering*. In some situations, it is possible the target IP or the IP of its trail re-appears in a partial route towards said target; not correctly identifying these cycles can lead to a neighborhood having its own identifying IP or an alias of it as peer(s), which is a problem for accurate topology inference. In measurements from September 2019, this issue usually occurs with less than 1% of inferred neighborhoods, but can be more widespread with some given vantage points/targets. Since the update, *self-peering* seems to have completely disappeared from the most recent measurements (November 2019).

* **January 7, 2020:** minor update of `WISE` v1.1 (only *Tool/*) to remove old comments and to refresh some pieces of code to ease the sharing of code between `WISE` and the new `SAGE`. A new parameter for configuration files as also been added (_aliasResolutionStrictMode_) and can be set to _True_ in order to force `WISE` to only consider aliases discovered with methods inherited from `Ally` and `iffinder` when performing alias resolution (these methods, when possible, are the most reliable).

## Disclaimer

`WISE` was written by Jean-François Grailet, currently Ph. D. student at the University of Liège (Belgium) in the Research Unit in Networking (RUN). Some parts of `WISE` re-uses code from an early version of `SAGE`. For more details on this version of `SAGE`, check its public repository:

https://github.com/JefGrailet/SAGE_beta

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/

Feel free to send me an e-mail if your encounter problems while compiling or running `WISE`. I am also inclined to answer questions regarding the algorithms used in `WISE` and to discuss its application in other research projects.
