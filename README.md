# WISE (Wide and lInear Subnet inferencE)

*By Jean-François Grailet (last updated: April 8, 2020)*

## Overview

`WISE` is a novel subnet inference tool which purpose is to overcome the limitations of state-of-the-art tools such as `TreeNET` and `ExploreNET`, in both accuracy and performance. In particular, it is able to complete its subnet inference in a time proportional to the amount of responsive IPs found within a target domain, which drastically reduces its execution time on large target networks.

`WISE` is designed such that it first completely analyzes the target network to both detect responsive IPs and collect some data on each (such as their respective distance as a minimal Time To Live value to get a proper reply) before conducting any subnet inference. All preliminary steps are accomplished in a linear time, but in practice, the data collection process uses some heuristics to speed up the whole process. If we except a short preliminary step which conducts alias resolution on a restricted set of IPs, the subnet inference itself is completely offline and is achieved by processing all discovered and analyzed IPs one by one, aggregating them in consecutive subnets (with respect to the address space).

`WISE` is currently only available for IPv4, but its design is arguably much better suited for IPv6 than previous subnet inference tools. It also comes as a 32-bit application (written in C/C++ for Linux distributions) to ensure compatibility with all testbed environments.

## About development

**`WISE` has been expanded into a new tool: [`SAGE`](https://github.com/JefGrailet/SAGE). Future updates of `WISE` will therefore mostly consist of minor fixes and adjustements that will also be applied to `SAGE`.**

It's worth noting that `WISE` is coded such that it can run on any machine, and in particular old environments. Indeed, `WISE` has been deployed almost exclusively on the PlanetLab testbed (which will be shut down at the end of May 2020, [as explained here](https://www.systemsapproach.org/blog/its-been-a-fun-ride)), where a lot of machines used to be 32-bit systems running with [Fedora 8](https://en.wikipedia.org/wiki/Fedora_(operating_system)). This is why it is still designed to be a 32-bit application and why it doesn't use features from newer C++ versions.

## Publications

`WISE` and its measurements are presented and discussed in two peer-reviewed publications. People wishing to get a big picture on the software free of implementation details are encouraged to read them.

* [Revisiting Subnet Inference WISE-ly](http://www.run.montefiore.ulg.ac.be/~grailet/docs/publications/WISE_TMA_2019.pdf)<br />
  Jean-François Grailet, Benoit Donnet<br />
  [Network Traffic Measurement and Analysis Conference (TMA) 2019](http://tma.ifip.org/2019/), Paris, 19/06/2017 - 21/06/2017

* [Virtual Insanity: Linear Subnet Discovery](http://www.run.montefiore.ulg.ac.be/~grailet/docs/publications/WISE_TNSM_2020.pdf)<br />
  Jean-François Grailet, Benoit Donnet<br />
  [IEEE Transactions on Network and Service Management](https://www.comsoc.org/publications/journals/ieee-tnsm) (see also on [IEEE Xplore](https://ieeexplore.ieee.org/document/9016121))

## Content of this repository

This repository consists of the following content:

* **Dataset/** provides datasets for various Autonomous Systems (or ASes) we measured with `WISE` from the PlanetLab testbed, from Fall 2018 to December 2019. Note that the earliest datasets only consist of IP dictionaries, as we used them to study our target networks and progressively design the final `WISE` v1.0.

* **Evaluation/** provides several sub-folders consisting of Python scripts written to build figures and the figures that were obtained on our public dataset. The purpose of each kind of figure is further described in additional README files. Two sub-folders in particular also briefly review how we validate `WISE` and compare it to `TreeNET` and `ExploreNET` and how we evaluate the _neighborhoods_ discovered with `WISE` v1.1.

* **v1/** provides all the source files of `WISE` v1.1, along some instructions to build and use it.

## Disclaimer

`WISE` was written by Jean-François Grailet, currently Ph. D. student at the University of Liège (Belgium) in the Research Unit in Networking (RUN).

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/

Feel free to send me an e-mail if your encounter problems while compiling or running `WISE`. I am also inclined to answer questions regarding the algorithms used in `WISE` and to discuss its application in other research projects.
