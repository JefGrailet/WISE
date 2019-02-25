# Trail analysis

*By Jean-François Grailet (last updated: February 25, 2019)*

## About

**N.B.:** for reminders, a _trail_ consists in an association between the last non-anonymous 
IP appearing in the route towards a target IP/subnet interface and the amount of hops between that 
hop and the target IP that are either cycles or anonymous hops. Ideally, a _trail_ should just be 
the last hop to emit a _Time exceeded_ message before getting a reply from the target IP while 
probing.

You can use the content of this folder to generate figures that depicts the proportion of trails 
(without anomalies) found in our datasets that suffers from one of the following issues:

* **flickering:** having a set of close consecutive IPs (w.r.t. the address space) that are 
  located at the same distance, if the trail IPs are alternating from one IP to another, then we 
  say the trail IPs are flickering.
* **warping:** the trail IP appears at various distances in terms of TTL (Time To Live).
* **echoing:** the trail IP is the same as the target IP (result of a specific router policy).

The script *GraphicalTrailAnalysis.py* will process a bunch of datasets for a given AS and a given 
set of dates to plot the evolution of the ratio of trails that are affected by one of the three 
issues listed above. The evolution is rather "by vantage point" rather than over time, since our 
datasets were collected on a daily basis but using a different vantage point at each measurement.

The script _TrailFigures.sh_ does the same job but over a set of ASes listed in an input file, in 
order to automatize the generation of figures over a whole campaign conducted from the PlanetLab 
testbed. It's worth noting the figures can be generated solely based on the _.ips_ output file 
produced by `WISE`, which is also why there are figures for a campaign spanning from November 2018 
to roughly the last third of December 2018 (at the time, `WISE` wasn't fully implemented and only 
scanned and analyzed responsive target IPs).

Here is the typical command you could use to generate figures for the ASes listed in 
_Example\_ASes_ for the dates given in _Example\_dates_:

```sh
./TrailFigures.sh Example\_ASes Example\_dates
```

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
