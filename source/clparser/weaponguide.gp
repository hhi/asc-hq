/* 
   This is the file from which the command line parsing code of ASC is 
   generated by genparse ( http://genparse.sourceforge.net ) 
*/
#description "generates html files that document ASCs units and buildings"
#mandatoryq  "vehicleFiles"
c / configfile  string            "Use given configuration file"
r / verbose     int 0 [0..10]     "Set verbosity level to x (0..10)"
d / directory   string            "place all output files in different directory"
l / linkdir     string            "relative directory for the menu links"
s / set         int 0 [0...]      "only use unitset with given ID"
i / image       flag              "generate images for units (Linux only)"
z / imgsize     int 48 [0...]     "size of unit images"
f / framename   string {"main"}   "name of the main frame"
t / style       string {"asc.css"} "name of the main pages' stylesheet"
m / menustyle   string {"asc.css"} "name of the menu stylesheet"
b / allbuildings flag             "generate doc for all buildings instead of uniquely named ones"
NONE / roottech string            "specify root technologies for tech dependency"
NONE / writeall flag              "skip the check for changed files, which speeds up operation, but touches all files"
