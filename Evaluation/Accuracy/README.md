# Subnet accuracy

*By Jean-François Grailet (last updated: January 7, 2020)*

## About

**N.B.:** if you reached this part of the repository via a direct link, please note that this 
sub-folder doesn't provide the data collected by `WISE`. You will rather find it in the 
*Dataset/* sub-folder located at the root of this repository.

You can use the content of this folder to generate figures depicting how many subnets were 
inferred in some target network and how many among them fullfill a certain amount of subnet 
rules, with a comparison across time (hence why one of the script is called 
*SubnetEvolution.py*). For reminders, the rules are the following:

* **Contra-pivot rule:** the given subnet features at least one contra-pivot interface, i.e., an 
  interface that appears sooner than all other interfaces and is most likely located on the router 
  providing access to the subnet.

* **Spread rule:** the given subnet features contra-pivot interfaces that are in the minority 
  among interfaces for large subnets, and not more numerous than regular (pivot) interfaces for 
  small subnets.

* **Outlier rule:** the given subnet has no interface that could be considered an outlier, i.e., 
  it does not seem to belong to the subnet at all.

There are two scripts: *RulesFigures.sh* and *SubnetEvolution.py*. The latter can be used _as is_ 
as long as you modify the variable `datasetPrefix` in the script to fit your own file system. The 
script *RulesFigures.sh* allows to run the same script for multiple ASes by providing the list of 
ASes and the list of dates for which the figures must be generated. The same script also 
automatically creates sub-folders to place the generated figures in.

Here is the typical command you could use to generate figures for the ASes listed in 
_Example\_ASes_ for the dates given in _Example\_dates_:

```sh
./RulesFigures.sh Example_ASes Example_dates
```

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
