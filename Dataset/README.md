# Dataset

*By Jean-François Grailet (last updated: July 3, 2020)*

## About

**Remark: in 2020, [PlanetLab will be shut down](https://www.systemsapproach.org/blog/its-been-a-fun-ride). The vantage points mentioned in our different snapshots will therefore no longer be available.**

The data provided in this repository consists in separate sets of files collected for a variety of 
Autonomous Systems (or ASes) at specific dates. Each set of files (or _snapshot_) was obtained by 
running a single instance of `WISE` (or an earlier version) on a single PlanetLab node. In order 
to renew the measurements in a more interesting manner, vantage points were rotated during each 
campaign. The dates of each campaign present in this dataset are provided below.

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

* Each AS sub-folder contains a target file suffixed in *.txt*. Such a file provides all IPv4 
  prefixes associated to the AS, with one prefix per line.

* The IPv4 prefixes were obtained via the BGP toolkit of Hurricane Electric. You can access 
  and use the BGP toolkit at the following address:
  
  http://bgp.he.net

* For some ASes, the target file has been updated in early February. If so, the previous target 
  file (usually generated at worst in 2017) is provided with an underscore (`_`) prefixing it.
  
* The builds of `WISE` deployed before February 19, 2019 provide an imperfect implementation of 
  subnet post-processing. In particular, this implementation forces /32 prefixes (typically, 
  loop-back interfaces) into /31 prefixes (typically, point-to-point links) due to an error while 
  translating the experimental subnet inference (initially written in Python) in the C/C++ 
  implementation of `WISE`. As a consequence, most measurements will contain many duplicate /31 
  subnets. Some other exotic scenarii also produced subnets overlapping the next one in the list. 
  Python scripts provided in other parts of this repository are written such that the overlapped 
  subnets are not taken into account for producing statistics and plotting figures. Any build of 
  `WISE` used after February 19, 2019 should not infer consecutive subnets overlapping each other.

* For the sake of reproducibility, we also provide in a **Scripts/** folder the bash scripts and 
  the typical files we used to schedule and retrieve our measurements.

## Composition of this dataset

For each AS sub-folder, you will find one or several *.txt* files listing the IPv4 prefixes 
retrieved with the BGP toolkit from Hurricane Electric along with sub-folders matching with the 
year and date of each measurement. Each unique set of files (or _snapshot_) is matched with a 
sub-path /yyyy/mm/dd/.

### Typical content

A dataset for a given autonomous system consists of several _snapshots_ which are collections of 
files computed during a single measurement of said AS from a single vantage point (or VP) using a 
single instance of `WISE`. Each snapshot is isolated by date of measurement and consists of 
several text files, the amount depending on the build of `WISE` used at the time of the 
measurement. Each file can be identified by its extension or full name.

**Remark (terminology):** an IP address is considered _lower_ when its 32-bit integer equivalent 
is lower than another address displayed in the same format. In practice, an IP address is lower 
than another when it contains the same first bytes then a lower byte (or when the first byte is 
lower). `1.2.3.5` is lower than both `1.2.3.6` and `2.2.3.5`.

### Typical content before December 28, 2018 

Snapshots collected before that date correspond to preliminary measurements used to design and 
calibrate the subnet inference of `WISE`. Inside, you will find the following files:

* **.ips file:** lists all IPs that were discovered during pre-scanning along their associated
  data (i.e., IP dictionary), with the _lowest_ IPs coming first. Special IPs (e.g. IPs which 
  appeared at different hop counts) are annotated with additional details.
* **VP.txt:** gives the host name of the **v**antage **p**oint used to measure the AS on that 
  specific date.

Note that the date of measurement is only the date at which the measurement was started. However,
most if not all measurements could be completed in less than 24 hours, hence why most measurements 
could be renewed on a daily basis with a different vantage point each day.

### Typical content between December 28, 2018 and February 28, 2019 (included)

Snapshots collected after that date are complete, i.e., they were obtained with a full 
implementation of `WISE` and provides all inferred subnets. Inside, you will find the following 
files:

* **.aliases-1 file:** contains all the aliases discovered during the preliminary step of subnet 
  inference (flickering IPs aliasing).
* **.fingerprints file:** contains all the fingerprints for each IP involved in alias resolution.
* **.hints file:** contains the alias resolution hints collected for each IP involved in alias 
  resolution.
* **.ips file:** lists all IPs that were discovered during pre-scanning along their associated
  data (i.e., IP dictionary), with the _lowest_ IPs coming first. Special IPs (e.g. IPs which 
  appeared at different hop counts) are annotated with additional details.
* **.subnets file:** lists all the subnets that were inferred along with their responsive 
  interfaces, ordered w.r.t. their prefix IP (i.e. _lowest_ prefix IP addresses first).
* **.txt file:** gives the details about how the measurement went, i.e., it gives the detailed 
  amount of probes used by each phase along the time they took for completion. This corresponds, 
  in fact, to the console output of `WISE`.
* **VP.txt:** gives the host name of the **v**antage **p**oint used to measure the AS on that 
  specific date.

In rare instances, lack of resources on the PlanetLab machine prevented the completion of the 
measurement, or the measurement had to be delayed. If this happened, an additional `README` file 
will appear in the folder and describe the issue.

Note that the date of measurement is only the date at which the measurement was started. However,
with a few exceptions (large and/or very responsive networks), `WISE` is usually capable of 
completing said measurement in less than 24 hours, hence why some ASes could be measured on a 
daily basis with a different vantage point each day.

### Typical content after September 4, 2019 (included)

Starting from September 4, 2019, we deployed `WISE` v1.1 on PlanetLab for new campaigns. This 
upgraded version produces the same output files as previously as well as new additional output 
files:

* **.peers file:** contains the results of the partial (Paris) `traceroute` measurements used to 
  perform neighborhood inference. Neighborhood inference is the new final stage of `WISE` v1.1; it 
  starts right after subnet inference.
* **.neighborhoods file:** lists all the neighborhoods that were inferred along with their 
  associated subnets, ordered w.r.t. their (first) label IP address (i.e. _lowest_ IPs first), 
  with the neighborhoods based on incomplete or echoing _trails_ coming first (_lowest_ IPs then 
  lowest anomaly or pre-echoing counts first). For reminders, a *neighborhood* is a network 
  location bordered by subnets located at at most one hop away from each other.

Just like before, the date of the measurement corresponds to the date at which the measurement was 
started, but measurements by `WISE` are usually completed within the next 24 hours except for 
very large and/or very responsive ASes.

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
