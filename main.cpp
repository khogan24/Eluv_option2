#include <iostream> 
#include <iomanip> 
#include <fstream> // io handling
#include <vector>
#include <string>
#include <utility> // pair
#include <bitset>
#include <sys/types.h>
#include <filesystem> // folder reading


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
	std::vector<std::pair<const std::string, int>> where;
	unsigned long key;
};

void swap(void* a, void*b)
{
	void* temp = a;
	b = a;
	a = temp;
}

/*
	finds and returns the longest common substring between 2 lists
	sets offset1 and offset2 to the respective locations of the first charachter in the strings
	an empty vector and both at 0 means no such string was found
		the only way for a binary file to do this is if one is all 1 and the other is all 0
	if 2 substring are of equal lenght, the first one found is returned
*/
std::vector<bool> longestCommonSubstring(std::vector<bool> * vec1, std::vector<bool> * vec2, int & offset1, int & offset2, size_t & key)
{
	int x = vec1->size();
	int y = vec2->size();
	if(!(x && y))// empty file
	{
		offset1 = -1;
		offset2 = -1;
		return std::vector<bool>();
	}
	
	std::vector<bool>* s1 = vec1, *s2 = vec2;
	if(y>x)
	{
		swap((void*)x,(void*)y);
		swap((void*)s1,(void*)s2);
	}
	// allocate a table
	// int[][] table; 

	int **table = (int**)malloc(x * sizeof(int*));
	for(int i = 0; i < x; i++)
	{
	    table[i] = (int*)malloc(y * sizeof(int));
		table[i][0] = 0; // segfaults here consistently on sample.1 and sample.2 at i values of about 30181 and x of 30720 
	}

	// any string of length 0 has no commonn substring with any other strings, so set those indicies to 0
	for(int i = 0; i < y; ++i)
	{
		table[0][i] = 0;
	}

	int s1_offset = 0, s1_temp = 0, s2_offset = 0, s2_temp = 0, len = 0, len_temp = 0;
	
	// loop through both string comapring elements
	for(int s1_index = 1; s1_index < x; ++s1_index)
	{
		for(int s2_index = 1; s2_index < y; ++s2_index)
		{
			// a substring is only common if both strings contain the same substring of chars
			// if any are not the same, the substring ends
			if(s1->at(s1_index) == s2->at(s2_index))
			{
				table[s1_index][s2_index] = table[s1_index - 1][s2_index - 1] + 1;
				len_temp = table[s1_index][s2_index];
				if(s1_temp == 0)
				{
					s1_temp = s1_index;
				}
				if(s2_temp == 0)
				{
					s2_temp = s2_index;
				}
			}
			else
			{
				//reset
				table[s1_index][s2_index] = 0;
				len_temp = 0;
				s1_temp = 0;
				s2_temp = 0;
			}
			if(len_temp > len)//
			{
				len = len_temp;
				s1_offset = s1_temp;
				s2_offset = s2_temp;
			}
		}
	}

	//free the mallocs
	for(int i = 0; i < x; ++i)
	{
	    free(table[i]);
	}free(table);

	offset1 = s1_offset;
	offset2 = s2_offset;
	std::vector<bool> returnVal = std::vector<bool>(s1->begin() + s1_offset, s1->begin() + (offset1 + len) );
	key = std::hash<std::vector<bool>>{}(returnVal);
	return returnVal;
}

/*
	Opens and reads binary file at path_to_file 
	returns contents in a vector<bool>
	returns empty vector if no such file is found
	sets the success value to 1 if read properly, and 0 on error
*/
std::vector<bool>* readFile( const std::string path_to_file, int& success )
{
	std::ifstream file( path_to_file, std::ios::binary ) ; // open in binary mode
	if(!file.good())
	{
		std::cout<< "No file '" << path_to_file << "' found, skipping..." << std::endl;
		file.close();
		success = 0;
		return nullptr;//std::vector<bool>(); // return empty vector and try rest of files
	}

    std::vector<bool>* bitString = new std::vector<bool>() ;
    char c ;
    while( file.get(c) ) // read byte by byte
	{
		bitString->push_back(  (unsigned int)(c) ); // cast to an u_int then send to list as bool
	}
    

	file.close();
	success = 1;
    return bitString ;
}



// returns contents of folder as a list of filenames
std::vector<std::string> folderContents(std::string folder)
{
	if(!std::filesystem::is_directory(folder))
	{
		std::cout << "No folder '" << folder << "' found, exiting." << std::endl;
		exit(1);
	}
	std::vector<std::string> list;
	for(const auto & entry : std::filesystem::directory_iterator(folder))
	{
		list.push_back( entry.path().string());
	}
	return list;
}

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, " error no folder given \n");
		fprintf(stderr, "usage is lcss <foldername>\n");
		return 1;
	}
	std::cout<< "opening folder: " << argv[1] << std::endl;
	std::vector<std::string> dir = folderContents(argv[1]);
	if(dir.empty())
	{
		fprintf(stderr, "empty folder, exiting \n");
		return 1;
	}
	printf("FOLDER CONTENTS\n");
	for(std::string i : dir)
	{
		std::cout << i << std::endl;
	}
	lcss currentLongest = {
		0,std::vector<bool>(),std::vector<std::pair<const std::string, int>>() ,(unsigned long) 0
	};
	int offset1 = 0, offset2 = 0, success1 = 0, success2 = 0;
	size_t key = 0;
	for(size_t i = 0; i < dir.size(); ++i)
	{	
		std::string s1 = dir[i];
		std::vector<bool> *file1 = readFile(s1, success1);
		for(size_t j = 0; j < i; ++j)// find if any new lcss are made by the addition of the new file
		{
			
			std::string s2 = dir[j];
			std::cout << "s1: " << i << " " << s1 << " s2: " << j <<" " << s2 << std::endl;
			
			std::vector<bool> *file2 = readFile(s2, success2);
			if(!success1 && !success2)
			{
				std::cout<<"there was an error in opening either " << s1 << " or " << s2 << std::endl;
				return 1;
			}
			std::vector<bool> curSubString = longestCommonSubstring(file1, file2, offset1, offset2, key);
			
			if(key == currentLongest.key ) //already seen string
			{
				currentLongest.where.push_back(std::make_pair(s1,offset2));// change to hashmap? so only one er file entry.
			}
			else if(curSubString.size() > currentLongest.substring.size())// new longest
			{
				currentLongest.substring = curSubString;
				currentLongest.key = key;
				currentLongest.length = curSubString.size();
				currentLongest.where = std::vector<std::pair<const std::string,int>>();
				currentLongest.where.push_back(std::make_pair(s1,offset1));
				currentLongest.where.push_back(std::make_pair(s2,offset2));
			}
		}
	}

	std::cout << "lenght : " << currentLongest.length << " key : " << currentLongest.key << " # of files " << currentLongest.where.size() << std::endl;
	std::cout << " substring : ";
	for(size_t i = 0 ; i < currentLongest.substring.size(); ++i)
	{
		std::cout << " " << currentLongest.substring.at(i); 
	}
}