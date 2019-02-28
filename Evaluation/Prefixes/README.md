# Prefix length distribution

*By Jean-François Grailet (last updated: February 28, 2019)*

## About

**N.B.:** if you reached this part of the repository via a direct link, please note that this 
sub-folder doesn't provide the data collected by `WISE`. You will rather find it in the 
*Dataset/* sub-folder located at the root of this repository.

You can use the content of this folder to generate figures showing the distribution of the subnet 
prefix length in a given dataset. A figure is generated for each date, rather than for a selection 
of dates (unlike other kinds of figures provided in this repository). It's worth noting that 
`WISE` stops growing a subnet beyond the /20 prefix length, hence why the X axis on each figure 
goes from /20 prefix to /32.

There are two scripts: *PrefixesFigures.sh* and *SubnetPrefixDistribution.py*. The latter can be 
used _as is_ as long as you modify the variable `datasetPrefix` in the script to fit your own file 
system. The script *PrefixesFigures.sh* allows to run the same script for multiple ASes by 
providing the list of ASes and the list of dates for which the figures must be generated. The same 
script also automatically creates sub-folders to place the generated figures in.

Here is the typical command you could use to generate figures for the ASes listed in 
_Example\_ASes_ for the dates given in _Example\_dates_:

```sh
./PrefixesFigures.sh Example_ASes Example_dates
```

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
