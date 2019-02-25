# Subnet persistence

*By Jean-François Grailet (last updated: February 25, 2019)*

## About

You can use the content of this folder to generate figures depicting the _persistence_ of subnets, 
i.e., given a reference dataset, the prefixes it contains re-appear in datasets measured on 
different days from a different vanatage point.

There are _persistent_ and _strictly persistent_ subnets. The latter denotes the subnets which 
fulfills the same amount of subnet rules (see below) as in the original dataset, while the former 
denotes the subnets for which only the prefix is the same (but there's a different amount of 
fulfilled rules).

For reminders, the rules are the following:

* **Contra-pivot rule:** the given subnet features at least one contra-pivot interface, i.e., an 
  interface that appears sooner than all other interfaces and is most likely located on the router 
  providing access to the subnet.

* **Spread rule:** the given subnet features contra-pivot interfaces that are in the minority 
  among interfaces for large subnets, and not more numerous than regular (pivot) interfaces for 
  small subnets.

* **Outlier rule:** the given subnet has no interface that could be considered an outlier, i.e., 
  it does not seem to belong to the subnet at all.

There are two scripts: *PersistenceFigures.sh* and *SubnetPersistence.py*. The latter can be used 
_as is_ as long as you modify the variable `datasetPrefix` in the script to fit your own file 
system. The script *PersistenceFigures.sh* allows to run the same script for multiple ASes by 
providing the list of ASes and the list of dates for which the figures must be generated. The same 
script also automatically creates sub-folders to place the generated figures in.

Here is the typical command you could use to generate figures for the ASes listed in 
_Example\_ASes_ for the dates given in _Example\_dates_:

```sh
./PersistenceFigures.sh Example\_ASes Example\_dates
```

Note that the reference dataset is, by default, the first dataset chronologically. In some 
situations, using another dataset as a reference might make more sense. You can trick the scripts 
by just interverting dates in the dates file (scripts don't check the dates are provided in 
chronological order).

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
