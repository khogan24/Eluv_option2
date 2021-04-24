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

// size_t** table;// i make this global to minimize malloc/free calls

struct TABLE
{
	size_t** arr;
	size_t x,y;
	int init = 0;
} table;

// this struct holds the byte array of the file, the file's name, and its index in the larger file_t array
struct file_t
{
	std::string name;
	std::vector<int> data;
};

// the current longest substring found in multiple files
// contains the string, a list of files, and their offsets
// and a key used to quickly tell if 2 substrings are the same
struct lcss
{
	int length;
	std::vector<int> substring;
	std::vector<std::pair<const std::string, int>> where;
	unsigned long key;
};

void swap(void* a, void*b)
{
	void* temp = a;
	b = a;
	a = temp;
}




//
size_t hashVec(std::vector<int> v)
{
	size_t seed = v.size();
	for(auto& i : v)
	{
		seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

void freeTable()
{
	for(int i = 0; i < table.x; ++i)
	{
	    free(table.arr[i]);
	}free(table.arr);
}

// resize table
void growTable(int x,int y)
{
	table.arr = (size_t**)malloc(x * sizeof(size_t*));
	for(int i = 0; i < x; ++i)
	{
		table.arr[i] = (size_t*)malloc(y * sizeof(size_t));
	}
}

// sets the global var table to be ready for use in the lcss algoritm
void initTable(int x, int y)
{
	if(!table.init)
	{
		table.x = x;
		table.y = y;
		growTable(x,y);
	}
	if(x > table.x || y > table.y)// need to grow
	{
		freeTable();
		growTable(x,y);
	}
	for( int i = 0; i < x; ++i)
	{
		table.arr[i][0] = 0;
	}
	for( int i = 0; i < y; ++i)
	{
		table.arr[0][i] = 0;
	}

}

/*
	finds and returns the longest common substring between 2 lists
	sets offset1 and offset2 to the respective locations of the first charachter in the strings
	an empty vector and both at 0 means no such string was found
		the only way for a binary file to do this is if one is all 1 and the other is all 0
	if 2 substring are of equal lenght, the first one found is returned
*/
std::vector<int> longestCommonSubstring(std::vector<int> * vec1, std::vector<int> * vec2, int & offset1, int & offset2, size_t & key)
{
	int x = vec1->size();
	int y = vec2->size();
	if(!(x && y))// empty file
	{
		offset1 = -1;
		offset2 = -1;
		return std::vector<int>();
	}
	
	std::vector<int>* s1 = vec1, *s2 = vec2;
	initTable(x,y);

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
				table.arr[s1_index][s2_index] = table.arr[s1_index - 1][s2_index - 1] + 1;
				len_temp = table.arr[s1_index][s2_index];
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
				table.arr[s1_index][s2_index] = 0;
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
	freeTable();

	offset1 = s1_offset;
	offset2 = s2_offset;
	std::vector<int> returnVal = std::vector<int>(s1->begin() + s1_offset, s1->begin() + (offset1 + len) );
	key = hashVec(returnVal);
	return returnVal;
}

/*
	Opens and reads binary file at path_to_file 
	returns contents in a vector<int>
	returns empty vector if no such file is found
	sets the success value to 1 if read properly, and 0 on error
*/
std::vector<int> readFile( const std::string path_to_file, int& success )
{
	std::ifstream file( path_to_file, std::ios::binary ) ; // open in binary mode
	if(!file.good())
	{
		std::cout<< "No file '" << path_to_file << "' found, skipping..." << std::endl;
		file.close();
		success = 0;
		return std::vector<int>(); // return empty vector and try rest of files
	}

    std::vector<int> bitString = std::vector<int>() ;
    char c ;
    while( file.get(c) ) // read byte by byte
	{
		bitString.push_back(  (unsigned int)(c) ); // cast to an u_int then send to list as bool
	}
	file.close();
	success = 1;
	std::cout << " file size: " << bitString.size() << std::endl;
    return bitString ;
}



// returns contents of all files in a folder as a list of said contents
std::vector<file_t>* folderContents(std::string folder)
{
	if(!std::filesystem::is_directory(folder))
	{
		std::cout << "No folder '" << folder << "' found, exiting." << std::endl;
		exit(1);
	}
	int success1 = 0;
	std::vector<file_t>* list = new std::vector<file_t>();
	printf("FOLDER CONTENTS\n");
	for(const auto & entry : std::filesystem::directory_iterator(folder))
	{
		std::cout << entry.path().string() << std::endl;
		file_t temp;
		temp.name = entry.path().string();
		temp.data = readFile(temp.name, success1);
		list->push_back( temp);
		if(!success1)
		{
			std::cout << "Error opening " << entry.path().string() << std::endl;
		}
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
	std::vector<file_t>* dir = folderContents(argv[1]);
	if(dir->empty())
	{
		fprintf(stderr, "empty folder, exiting \n");
		return 1;
	}
	lcss currentLongest = {
		0,std::vector<int>(),std::vector<std::pair<const std::string, int>>() ,(unsigned long) 0
	};
	int offset1 = 0, offset2 = 0, success2 = 0;
	size_t key = 0;
	for(size_t i = 0; i < dir->size(); ++i)
	{	
		
		std::vector<int> *file1 = &(dir->at(i).data);
		for(size_t j = 0; j < i; ++j)// find if any new lcss are made by the addition of the new file
		{
			
			std::vector<int> *file2 = &(dir->at(i).data);
			std::cout << i << " - " << j << std::endl;

			std::vector<int> curSubString = longestCommonSubstring(file1, file2, offset1, offset2, key);
			
			if(key == currentLongest.key ) //already seen string
			{
				currentLongest.where.push_back(std::make_pair( dir->at(i).name ,offset2));// change to hashmap? so only one er file entry.
			}
			else if(curSubString.size() > currentLongest.substring.size())// new longest
			{
				currentLongest.substring = curSubString;
				currentLongest.key = key;
				currentLongest.length = curSubString.size();
				currentLongest.where = std::vector<std::pair<const std::string,int>>();
				currentLongest.where.push_back(std::make_pair(dir->at(i).name,offset1));
				currentLongest.where.push_back(std::make_pair(dir->at(j).name,offset2));
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