# WISE (Wide and lInear Subnet inferencE)

*By Jean-François Grailet (last updated: February 25, 2019)*

## Overview

`WISE` is a novel subnet inference tool which purpose is to overcome the limitations of state-of-the-art tools such as `TreeNET` and `ExploreNET`, in both accuracy and performance. In particular, it is able to complete its subnet inference in a time proportional to the amount of responsive IPs found within a target domain, which drastically reduces its execution time on large target networks.

`WISE` is designed such that it first completely analyzes the target network to both detect responsive IPs and collect some data on each (such as their respective distance as a minimal Time To Live value to get a proper reply) before conducting any subnet inference. All preliminary steps are accomplished in a linear time, but in practice, the data collection process uses some heuristics to speed up the whole process. If we except a short preliminary step which conducts alias resolution on a restricted set of IPs, the subnet inference itself is completely offline and is achieved by processing all discovered and analyzed IPs one by one, aggregating them in consecutive subnets (with respect to the address space).

`WISE` is currently only available for IPv4, but its design is arguably much better suited for IPv6 than previous subnet inference tools. It also comes as a 32-bit application (written in C/C++ for Linux distributions) to ensure compatibility with all PlanetLab computers.

## About development

Future updates of `WISE` could include:

* Improved subnet post-processing.
* 64-bit version and/or IPv6 version.

## Content of this repository

This repository consists of the following content:

* **Dataset/** provides datasets for various Autonomous Systems (or ASes) we measured with `WISE` from the PlanetLab testbed. Note that the earliest datasets only consist of IP dictionaries, as we used them to study our target networks and progressively design the final version of `WISE` v1.0.

* **Evaluation/** provides several sub-folders consisting of Python scripts written to build figures and the figures that were obtained on our public dataset. The purpose of each kind of figure is further described in additional README files. A sub-folder in particular also briefly review how we validate `WISE` and compare it to `TreeNET` and `ExploreNET`.

* **v1/** provides all the source files of `WISE` v1.0, along some instructions to build and use it.

## Disclaimer

`WISE` was written by Jean-François Grailet, currently Ph. D. student at the University of Liège (Belgium) in the Research Unit in Networking (RUN).

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/

Feel free to send me an e-mail if your encounter problems while compiling or running `WISE`. I am also inclined to answer questions regarding the algorithms used in `WISE` and to discuss its application in other research projects.
