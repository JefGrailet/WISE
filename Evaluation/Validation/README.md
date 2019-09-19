# Validating WISE (and comparing it to other tools)

*By Jean-François Grailet (last updated: September 19, 2019)*

## About

This folder provides a script *SubnetValidation.py* which can be used for several purposes:

* assessing the amount of subnet prefixes discovered by `WISE` that matches a groundtruth,
* seeing how close to the truth other discovered prefixes are, 
* comparing `WISE` to `TreeNET` and `ExploreNET`.

The validation takes the form of a figure where the Y axis shows a ratio of subnets of the 
groundtruth that are matched, while the X axis shows the difference in prefix length between an 
inferred subnet and the groundtruth subnet. In the best of the worlds, 100% of the inferred 
subnets should be located at the place where the blue bar is located, in the middle of the figure, 
as it indicates that all inferred subnets matches perfectly the groundtruth.

Otherwise, the left side of the figure corresponds to overgrown subnets, while the right side 
corresponds to undergrown subnets. If the tool doesn't match exactly all prefixes, it is 
desirable that the curve appears more elevated on the right side of the figure (especially at 
1 and 2) rather than the left side for two reasons:

* because of lack of responsive interfaces spread in its whole address space, a subnet can be 
  naturally undergrown with respect to the true prefix, 
* if the true subnet appears to be chunked in several parts, then some post-processing could 
  help to re-build it as a single unified subnet.

In order to compare `WISE` to `TreeNET` and `ExploreNET`, our suggestion is to compile and run 
`SAGE`. `SAGE` is a more advanced version of `TreeNET` which is able to infer subnets the same 
way as `TreeNET` but which is also able to output subnets as they would have been inferred by
`ExploreNET` alone. One can then obtain subnets as inferred by `TreeNET` and `ExploreNET`, 
respectively, by running this command:

```sh
sudo ./sage Targets.txt -l My_data -j
```

Subnets as discovered by `TreeNET` will be found in *My_data.subnets* while subnets as inferred 
by `ExploreNET` will be in *My_data.xnet*. To run `SAGE` in background while outputting the 
largest parts of the console output into external logs, one can use this command instead:

```sh
sudo ./sage Targets.txt -l My_data -j -k > My_data.txt 2>&1 &
```

You can get `SAGE` (source code, measurements, etc.) at the following URL:

https://github.com/JefGrailet/sage

Finally, it's worth noting that:

* the groundtruth should be provided as a list of subnet prefixes (one per line), 
* the *SubnetValidation.py* script doesn't need edition and can be used _as is_, but requires all 
  at once the groundtruth prefixes, the subnets as inferred by `WISE`, `TreeNET` and `ExploreNET` 
  (respectively).

**Update (September 19, 2019):** the validation now can compare subnet prefixes obtained by `WISE` 
before and after being post-processed to be adjusted in size. By default, the validation script 
compares *raw* prefixes (i.e., before post-processing), but appending the command-line with 
*--adjusted* will make it use the adjusted prefixes instead.

## Contact

**E-mail address:** Jean-Francois.Grailet@uliege.be

**Personal website:** http://www.run.montefiore.ulg.ac.be/~grailet/
