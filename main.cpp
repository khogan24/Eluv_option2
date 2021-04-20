#include <iostream> 
#include <iomanip> 
#include <fstream> // io handling
#include <vector>
#include <string>
#include <utility> // pair
#include <bitset>
#include <dirent.h>
#include <sys/types.h>
// #include <filesystem>


#define MAX(a,b) (a <= b) ? a:b
// this struct holds the byte array of the file, the file's name, and its index in the larger file_t array
struct file_t
{
	char* name;
	std::vector<bool> data;
};

// the current longest substring found in multiple files
// contains the string, a list of files, and their offsets
// and a key used to quickly tell if 2 substrings are the same
struct lcss
{
	int length;
	std::vector<bool> substring;
	std::vector<std::pair<const char*, int>> where;
	unsigned long key;
};



/*
	finds and returns the longest common substring between 2 lists
	sets offset1 and offset2 to the respective locations of the first charachter in the strings
	an empty vector and both at 0 means no such string was found
		the only way for a binary file to do this is if one is all 1 and the other is all 0
	if 2 substring are of equal lenght, the first one found is returned
*/
std::vector<bool> longestCommonSubstring(const std::vector<bool> & s1, const std::vector<bool> & s2, int & offset1, int & offset2, size_t & key)
{
	if(!(s1.size() && s2.size()))// empty file
	{
		offset1 = -1;
		offset2 = -1;
		return std::vector<bool>();
	}
	int x = s1.size();
	int y = s2.size();
	// allocate a table
	// int[][] table; 

	int **table = (int**)malloc(x * sizeof(int*));
	for(int i = 0; i < y; i++)
	{
	    table[i] = (int*)malloc(y * sizeof(int));
	}


	// any srting of length 0 has no commonn substring with any other strings, so set those indicies to 0
	// fprintf(stderr, "s1.size :%d",x);
	for(int i = 0; i < x; ++i)
	{
		table[0][i] = 0;
	}
	std::cout << "second" << std::endl;
	for(int i = 0; i < y; ++i)
	{
		table[0][i] = 0;
	}
	std::cout << "after 2nd" << std::endl;

	int s1_offset = 0, s1_temp = 0, s2_offset = 0, s2_temp = 0, len = 0, len_temp = 0;
	
	// loop through both string comapring elements
	for(int s1_index = 1; s1_index < x; ++s1_index)
	{
		for(int s2_index = 1; s2_index < y; ++s2_index)
		{
			// a substring is only common if both strings contain the same substring of chars
			// if any are not the same, the substring ends
			if(s1[s1_index] == s2[s2_index])
			{
				// increment length of sub-string
				// std::cout << s1_index << ":" << s2_index << std::endl;
				table[s1_index][s2_index] = table[s1_index - 1][s2_index - 1] + 1;
				len_temp = table[s1_index][s2_index];// record length

				// upon reaching a new substring, record the offsets
				if(s1_temp == 0) s1_temp = s1_index;
				if(s2_temp == 0) s2_temp = s2_index;
		
			}
			else
			{
				// mismatch, set element to 0
				table[s1_index][s2_index] = 0;
				// we will see a mismatch at the end of the substring, if new substring is longer, update values
				if(len_temp > len)
				{
					s1_offset = s1_temp;
					s2_offset = s2_temp;
					len = len_temp;
				}
				// reset
				len_temp = 0;
				s1_temp = 0;
				s2_temp = 0;
			}
		}
		len = MAX(len, len_temp);
	}

	//free the mallocs
	for(int i = 0; i < y; i++)
	{
	    free(table[i]);
	}free(table);

	offset1 = s1_offset;
	offset2 = s2_offset;
	std::vector<bool> returnVal = std::vector<bool>(s1.begin() + s1_offset, s1.begin() + (offset1 + len) );
	key = std::hash<std::vector<bool>>{}(returnVal);
	return returnVal;

}

/*
	Opens and reads binary file at path_to_file 
	returns contents in a vector<bool>
	returns empty vector if no such file is found
	sets the success value to 1 if read properly, and 0 on error
*/
std::vector<bool> readFile( const char* path_to_file, int& success )
{
	std::ifstream file( path_to_file, std::ios::binary ) ; // open in binary mode
	if(!file.good())
	{
		std::cout<< "No file '" << path_to_file << "' found, skipping..." << std::endl;
		file.close();
		success = 0;
		return std::vector<bool>(); // return empty vector and try rest of files
	}

    std::vector<bool> bitString = std::vector<bool>() ;// small size
    char c ;
    while( file.get(c) ) // read byte by byte
	{
		bitString.push_back(  (unsigned int)(c) ); // cast to an u_int then send to list as bool
	}
    

	file.close();
	success = 1;
    return bitString ;
}



// gets contents of folder as a list of filenames
std::vector<char*> folderContents(char* folder)
{
	struct dirent *entry;
	DIR *dir = opendir(folder);

	if (dir == NULL) 
	{
		fprintf(stderr, "directory %s not found", folder);
		return std::vector<char*>();
	}

	std::vector< char* > contents = std::vector<char*>();
	while ((entry = readdir(dir)) != NULL) 
	{
		std::cout << entry->d_name << std::endl;
		contents.push_back(entry->d_name);
	}
	closedir(dir);
	return contents;
}

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, " error no folder given \n");
		fprintf(stderr, "usage is ./lcss <foldername(s)>\n");
		return 1;
	}
	char* folderName = argv[1];
	printf("opening folder: %s \n", folderName);
	std::vector<char*> dir = folderContents(folderName);
	if(dir.empty())
	{
		fprintf(stderr, "empty folder, exiting \n");
		return 1;
	}
	printf("FOLDER CONTENTS\n");
	for(char* i : dir)
	{
		printf("%s\n", i);
	}
	// TODO add flags? maybe
	lcss currentLongest = {
		0,std::vector<bool>(),std::vector<std::pair<const char*, int>>() ,(unsigned long) 0
	};
	int offset1 = 0, offset2 = 0, error1 = 0, error2 = 0;
	size_t key = 0;
	for(int i = 0; i < dir.size(); ++i)
	{	
		for(int j = 0; j < i -1 ; ++j)// find if any new lcss are made by the addition of the new file
		{
			std::string s1 = dir[i];
			std::string s2 = dir[j];
			std::cout << "s1: " << s1 << " s2: " << s2 << std::endl;
			std::vector<bool> curSubString = longestCommonSubstring(readFile((folderName + s1).c_str(), error1), readFile((folderName + s2).c_str(), error2), offset1, offset2, key);
			if(error1 && error2)
			{
				fprintf(stderr, "there was an error in opening either %s or %s \n ... exiting\n", s1,s2);
				return 1;
			}
			if(key == currentLongest.key ) //already seen string
			{
				currentLongest.where.push_back(std::make_pair(s1.c_str(),offset2));// change to hashmap? so only one er file entry.
			}
			else if(curSubString.size() > currentLongest.substring.size())// new longest
			{
				currentLongest.substring = curSubString;
				currentLongest.key = key;
				currentLongest.length = curSubString.size();
				currentLongest.where = std::vector<std::pair<const char*,int>>();
				currentLongest.where.push_back(std::make_pair(s1.c_str(),offset1));
				currentLongest.where.push_back(std::make_pair(s2.c_str(),offset2));
			}
		}
	}

	std::cout << "lenght : " << currentLongest.length << " key : " << currentLongest.key << " # of files " << currentLongest.where.size() << std::endl;
	std::cout << " substring : ";
	for(int i = 0 ; i < currentLongest.substring.size(); ++i)
	{
		std::cout << " " << currentLongest.substring.at(i); 
	}
}