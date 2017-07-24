# SPA
Slack Space Analyzer


Since the beginning of mankind that the human being had the need to hide
information and with that need many methods were developed, some more
effective than others. With the appearance of computers, more effective
methods have emerged, such as hiding information in the file’s slack space.
This is the focus of our work. We chose to focus only on one of the tools that
hides information in the file’s slack space, the BMAP. Our tool, called SPA
(Slack sPace Analyzer), aims to reveal the information hidden by BMAP in
a file. In addition to this main functionality, we also have the possibility
to analyze a directory and its sub directories for files that have information
in their slack space. We can also delete the information stored in the slack
space of one or more files.

Instructions:




To run SPA, you will need a 32-bit Linux system (a simple 32-bit VM will do).

Proceed by extracting the files and change directory to the /bmap folder.

Type "make", and now you should be read to run bmap tool.

To compile SPA, type in your console "gcc -o spa spafinal.c"

For test purposes, create a simple text file and hide whatever information you want (example):



echo "message" | ./bmap --mode putslack textfilename
run "./spa -help" for all command options
