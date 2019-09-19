# Subnet persistence

*By Jean-François Grailet (last updated: September 19, 2019)*

## About

**N.B.:** if you reached this part of the repository via a direct link, please note that this 
sub-folder doesn't provide the data collected by `WISE`. You will rather find it in the 
*Dataset/* sub-folder located at the root of this repository.

You can use the content of this folder to generate figures depicting the _persistence_ of subnets, 
i.e., given a reference dataset, the prefixes it contains re-appear in datasets measured on 
different days from a different vanatage point.

There are _(weakly) persistent_ and _strictly persistent_ subnets. The latter denotes the subnets 
which fulfills the same amount of subnet rules (see below) as in the original dataset, while the 
former denotes the subnets for which only the prefix is the same (but there's a different amount 
of fulfilled rules).

For reminders, the rules are the following:

* **Contra-pivot rule:** the given subnet features at least one contra-pivot interface, i.e., an 
  interface that appears sooner than all other interfaces and is most likely located on the router 
  providing access to the subnet.

* **Spread rule:** the given subnet features contra-pivot interfaces that are in the minority 
  among interfaces for large subnets, and not more numerous than regular (pivot) interfaces for 
  small subnets.

* **Outlier rule:** the given subnet has no interface that could be considered an outlier, i.e., 
  it does not seem to belong to the subnet at all.

This folder provides three scripts: *PersistenceFigures.sh*, *SubnetPersistence.py* and 
*WeaklyPersistentSubnets.py*.

*PersistenceFigures.sh* and *SubnetPersistence.py* are to used to generated figures showing the 
persistence of subnets over time. The latter can be used _as is_ as long as you modify the 
variable `datasetPrefix` in the script to fit your own file system. The script 
*PersistenceFigures.sh* allows to run the same script for multiple ASes by providing the list of 
ASes and the list of dates for which the figures must be generated. The same script also 
automatically creates sub-folders to place the generated figures in.

Here is the typical command you could use to generate figures for the ASes listed in 
_Example\_ASes_ for the dates given in _Example\_dates_:

```sh
./PersistenceFigures.sh Example_ASes Example_dates
```

*WeaklyPersistentSubnets.py*, on the other hand, can be used to list the subnets which are weakly 
persistent, i.e., persistent but not fulfilling the same set of rules. This script can be used, 
for instance, to compare different measurements of a same subnet across time and observe the 
effects of changing the vantage point and having more or less issues with traffic engineering. The 
subnets are listed in the console for each date. Like *SubnetPersistence.py*, the variable 
`datasetPrefix` should be edited before running the script.

Note that for both Python scripts, the reference dataset is, by default, the first dataset 
chronologically. In some situations, using another dataset as a reference might make more sense. 
You can trick the scripts by just interverting dates in the dates file (scripts don't check the 
dates are provided in chronological order).

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
