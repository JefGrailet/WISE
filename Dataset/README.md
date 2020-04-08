# Dataset

*By Jean-François Grailet (last updated: April 8, 2020)*

## About

**Remark: in late May 2020, [PlanetLab will be shut down](https://www.systemsapproach.org/blog/its-been-a-fun-ride). The vantage points mentioned in our different snapshots will therefore no longer be available.**

The data provided in this repository consists in separate datasets provided for a variety of 
Autonomous Systems (or ASes) for specific dates. Each dataset was obtained by running a single 
instance of `WISE` (or an earlier version) on a single PlanetLab node. In order to renew the 
measurements in a more interesting manner, vantage points were rotated during each campaign. The 
dates of each campaign present in this dataset are provided below.

|  Start date  |  End date  |  # probed ASes  |  Number  |
| :----------: | :--------- | :-------------- | :------- |
| 29/11/2018   | 21/12/2018 | 22              | 1        |
| 28/12/2018   | 24/01/2019 | 22              | 2        |
| 01/02/2019   | 14/02/2019 | 10              | 3        |
| 01/02/2019   | 11/02/2019 | 4               | 4        |
| 19/02/2019   | 28/02/2019 | 10              | 5        |
| 19/02/2019   | 25/02/2019 | 4               | 6        |
| 04/09/2019   | 17/09/2019 | 13              | 7        |
| 16/09/2019   | 23/09/2019 | 4               | 8        |
| 12/11/2019   | 17/11/2019 | 5               | 9        |
| 19/11/2019   | 06/12/2019 | 12              | 10       |

The ASes targetted by each campaign as well as the sets of involved PlanetLab nodes can be found 
in the "CampaignX.md" files at the root of this folder, where X is the number of the campaign as 
shown in the above table.

A few remarks about this public dataset:

* Each AS sub-folder contains a target file suffixed in *.txt*. Such a file provides one IPv4
  prefix per line.

* The IPv4 prefixes were obtained via the BGP toolkit of Hurricane Electric. You can access 
  and use the BGP toolkit at the following address:
  
  http://bgp.he.net

* For some ASes, the target file has been updated in early February. If so, the previous target 
  file (usually generated at worst in 2017) is provided with an underscore (`_`) prefixing it.
  
* While the subnet post-processing phase is not definitive at the moment, the implementation of 
  `WISE` used before February 19, 2019 provides an imperfect implementation for it. 
  In particular, this implementation forces /32 prefixes (typically, loop-back interfaces) into 
  /31 prefixes (typically, point-to-point links) due to an error while translating the 
  experimental subnet inference (initially written in Python) in the C/C++ implementation of 
  `WISE`. As a consequence, most datasets will contain many duplicate /31 subnets. Some other 
  exotic scenarios also produced subnets overlapping the next one in the list. Python scripts 
  provided in other parts of this repository are written such that the overlapped subnets are 
  not taken into account for producing statistics and plotting figures. The implementation of 
  `WISE` after February 19, 2019 produces little to no overlapping subnets.

* For the sake of reproducibility, we also provide in a **Scripts/** folder the bash scripts and 
  the typical files we used to schedule and retrieve our measurements.

## Composition of each dataset

For each AS sub-folder, you will find one or several *.txt* files listing the IPv4 prefixes 
retrieved with the BGP toolkit from Hurricane Electric along with sub-folders matching the year 
and date of each measurement. Each unique dataset is matched with a sub-path /yyyy/mm/dd/. Then, 
in each sub-folder, you will find the content described below.

### Before December 28, 2018 

Datasets collected before that date correspond to preliminary measurements used to design and 
calibrate the subnet inference of `WISE`. Inside, you will find the following files:

* **.ips file:** lists all IPs that were discovered during pre-scanning along their associated
  data (i.e., IP dictionary). Special IPs (e.g. IPs which appeared at different hop counts) are 
  also annotated with additional details.
* **VP.txt:** gives the PlanetLab node (the **v**antage **p**oint) used to measure the AS on that 
  specific date.

Note that the date of measurement is only the date at which the measurement was started. However,
most if not all measurements could be completed in less than 24 hours, hence why most measurements 
could be renewed on a daily basis with a different vantage point each day.

### Between December 28, 2018 and February 28, 2019 (included)

Datasets collected after that date are complete, i.e., they were obtained with a full 
implementation of `WISE` and provides all inferred subnets. Inside, you will find the following 
files:

* **.aliases-1 file:** contains all the aliases discovered by `WISE` during the preliminary step 
  of subnet inference (flickering IPs aliasing).
* **.fingerprints file:** contains all the fingerprints for each IP involved in alias resolution.
* **.hints file:** contains the alias resolution hints collected for each IP involved in alias 
  resolution.
* **.ips file:** lists all IPs that were discovered during pre-scanning along their associated
  data (i.e., IP dictionary). Special IPs (e.g. IPs which appeared at different hop counts) are 
  also annotated with additional details.
* **.subnets file:** lists, in order, all the subnets that were inferred along their responsive
  interfaces and respective route.
* **.txt file:** gives the details about how the measurement went, i.e., it gives the detailed 
  amount of probes used by each phase along the time they took for completion.
* **VP.txt:** gives the PlanetLab node (the **v**antage **p**oint) used to measure the AS on that 
  specific date.

In some situations, lack of resources on the PlanetLab machine prevented the completion of the 
measurement, or the measurement had to be delayed. If this happened, an additional `README` file 
will appear in the folder and describe the issue.

Note that the date of measurement is only the date at which the measurement was started. However,
with a few exceptions (large and/or very responsive networks), `WISE` is usually capable of 
completing said measurement in less than 24 hours, hence why some ASes could be measured on a 
daily basis with a different vantage point each day.

### After September 4, 2019 (included)

Starting from September 4, 2019, we deployed `WISE` v1.1 on PlanetLab for new campaigns. This 
upgraded version produces the same output files as previously as well as new additional output 
files:

* **.peers file:** contains the results of the partial (Paris) `traceroute` measurements which 
  are performed by the new neighborhood inference stage (new final stage; starts after subnet 
  inference).
* **.neighborhoods file:** lists and details the neighborhoods inferred by `WISE` v1.1 during its 
  new final algorithmic step. For reminders, a *neighborhood* is a network location bordered by 
  subnets located at at most one hop away from each other.

Just like before, the date of the measurement corresponds to the date at which the measurement was 
started, but measurements by `WISE` are usually completed within the next 24 hours except for 
very large and/or very responsive ASes.

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
