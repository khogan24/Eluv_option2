# Eluv_option2
Finding the longest common substring between a number of files.

Use : lcss.exe <foldername>

lcss will try to examine every file in the given folder

Unfinished, segfaults at line 69, even though the i value is well below the x value

Written in c++17 for windows 10
support of std::filesystems is needed to compile

I open 2 files comparing the contents, then close both, and move to the next.
I check to see if the new file contains a new longest amoung the previosly open files
This reduces the memory used, at the cost of higher runtime constants ( needing to open an close files repeatedly)
If one had more memory, or could confirm small files, one could open all in memory and run the algorithm off those.
Runtime O(#files^2 * lcss)

The longest common substring algorithm (lcss) checks to see if both string share the same char, if so we increment an entry in a 2d table, and move to the next char of one string. upon reaching the end of one string, the other has it's index incremented and the process starts over.
Runtime O((size of largest file)^2)

