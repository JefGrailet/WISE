# Evaluating neighborhoods collected by WISE v1.1

*By Jean-François Grailet (last updated: November 8, 2019)*

## About

This folder provides some scripts to validate and more generally evaluate the neighborhoods 
discovered by `WISE`, using its 1.1 version (or more recent).

A _neighborhood_ is a concept used to make a first approximation of the topology of a target 
network. It has been already used in the past with `TreeNET` and `SAGE`, which you can learn more 
about by checking their dedicated repositories:

https://github.com/JefGrailet/treenet<br/>
https://github.com/JefGrailet/SAGE

A _neighborhood_ is formally defined as a network location bordered by a set of subnets which are 
located at at most one hop from each other. A neighborhood is typically inferred by looking at the 
last hop before the pivot interface(s) of each subnet and grouping subnets when they share the 
same last hop. In `WISE` terminology, this last hop is also called _trail_, and therefore a 
neighborhood in `WISE` is simply discovered by grouping subnets having the same trail (or 
flickering trails), with some exceptions.

`WISE` also introduces the notion of neighborhood _peer_, used to locate neighborhoods with 
respects to each others. Given a neighborhood _A_ with a trail _a_, _B_ is a peer of _A_ if 
and only if its trail _b_ appears right before _a_ in routes towards pivot interfaces of _A_. If 
there is no intermediate hop between _b_ and _a_, we also say that _B_ is a _direct peer_ of _A_.

This folder provides two scripts you can use to evaluate neighborhoods in general, using data 
produced by `WISE`.

* _NeighborhoodsAnalysis.py_ processes several datasets collected for a given AS to have a first 
look at the notion of _peer_ and evaluate its viability. In particular, it produces two figures, 
one giving the CDF of the distance between any neighborhood and its peer(s) (the X axis 
corresponding to TTL = 0, 1, 2...) and another one giving the PDF of the amount of peers found for 
the discovered neighborhoods in a log-log plot. Examples of such figures can be found within this 
repository (computed with datasets collected in September 2019). To simplify the generation of the 
figures, a bash script is provided in order to run _NeighborhoodsAnalysis.py_ on several ASes and 
put the results in automatically generated sub-directories. Here is the typical command you 
could use to generate all figures for the ASes listed in _Example\_ASes_ for the dates given in 
_Example\_dates_, the output figures being prefixed with _September2019_:

```sh
./PlotFigures.sh Example_ASes Example_dates September2019
```

* _NeighborhoodsValidation.py_ processes one _.neighborhoods_ file and a ground truth network 
described in text format to evaluate how often the prefixes common to both files are correctly 
identified w.r.t. the topology. More precisely, this script will classify a given pair of 
prefixes as a _true positive_ if they belong to the same router (i.e., in the ground truth file) 
and same neighborhood (i.e., as inferred by `WISE`), as a _false positive_ if they belong to a 
same neighborhood but not a same router, as a _true negative_ if they are disjoint in both files 
and as a _false negative_ if they are not found around the same neighborhood but are associated 
in practice to the same router. The routers should be provided as a succession of lines in the 
format `[prefix] [router_ID]` where the blank space is replaced with a tabulation, while the 
_.neighborhoods_ file is simply the file outputted by `WISE` 1.1 at the end of its execution. A 
third file can be provided to advertise the routers which are working as one during the 
measurements, i.e., meshes of routers. Each line of this file should consist of the IDs that 
correspond to a same mesh, separated by blank spaces. The results of the validation (i.e., 
_true positives_ rate, _true negatives_ rate, accuracy, etc.) are directly written in the 
terminal. The typical command line to run this script should look like this:

```sh
python NeighborhoodsValidation.py [ground truth.txt] [.neighborhoods file] [[routers_meshes.txt]]
```

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
