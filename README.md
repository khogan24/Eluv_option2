# Eluv_option2
Finding the longest common substring between a number of files.

Use : lcss.exe <foldername>
linux ./lcss <foldername>

lcss will try to examine every file in the given folder

Written in c++17
compiled with gcc - 9
support of std::filesystems is needed to compile

I open all files into memory to lower the runtime at the cost of memory.
Then I iterate over the list, checking the file at the current index.
I check to see if the new file contains a new longest amoung the previosly checked files
If one was restricted in memory, opening files comparing contents, and then closing would save space, but be slower.

Runtime O(#files^2 * lcss)

The longest common substring algorithm (lcss) checks to see if both string share the same char, if so we increment an entry in a 2d table, and move to the next char of one string. upon reaching the end of one string, the other has it's index incremented and the process starts over.

Runtime O((size of largest file)^2)

