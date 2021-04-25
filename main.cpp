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

//  a struct to hold an array with some meta data about said array
struct TABLE
{
	int* arr;
	int size;
	int init = 0;
};

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

//hashes a vector to provide faster comparisons
size_t hashVec(std::vector<int> v)
{
	size_t seed = v.size();
	for(int i = 0; i < seed; ++i)
	{
		seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

void freeTable(TABLE & table)
{
	free(table.arr);
}

// resize table and set all values to 0
void growTable(TABLE & table, int size)
{
	table.arr = ( int*)malloc(size * sizeof( int));
	table.size = size;
	table.init = 1;
	for(int i = 0; i < size; ++i)
	{
		// might not be needed depending on OS, but here just to be sure
		table.arr[i] = 0;
	}
}

// sets the var table to be ready for use in the lcss algoritm
// table will be filled with all 0's
void initTable(TABLE & table, int size)
{
	if(!table.init)
	{
		table.size = size;
		growTable(table, size);
	}
	if(size > table.size)// need to grow
	{
		freeTable(table);
		growTable(table, size);
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

	// swap so x is always bigger
	if(y > x)
	{
		int temp = x;
		x = y;
		y = temp;
		s1 = vec2;
		s2 = vec1;
	}
	
	TABLE table = TABLE{};
	initTable(table, x);

	int len = 0, tempLen = 0;
	for(int s1_index = 0; s1_index < x; ++s1_index)
	{
		for(int s2_index = 0; s2_index < s1_index; ++s2_index)
		{
			if(s1_index == 0)
			{
				table.arr[s1_index] = (s1->at(s1_index) == s2->at(s2_index) ? 1 : 0);
			}
			else table.arr[s1_index] = (s1->at(s1_index) == s2->at(s2_index)? (table.arr[s1_index - 1] + 1) : 0);
			if(table.arr[s1_index] > len )
			{
				len = table.arr[s1_index];
				offset1 = s1_index - len;
				offset2 = s2_index - len;
			}
		}
	}

	freeTable(table);
	std::vector<int> returnVal( s1->begin() + (offset1) , s1->begin() + (offset1 + len));
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
		bitString.push_back(  (unsigned int)(c) );
	}
	file.close();
	success = 1;
	std::cout << " file size: " << bitString.size() << std::endl;
	// std::cout << "exit" << std::endl;
	// exit(-1);
    return bitString ;
}

void printBinary(unsigned number){
   if (number > 1)
      printBinary(number/2);
   std::cout << number % 2;
}


// returns contents of all files in a folder as a list of said contents
std::vector<file_t> folderContents(std::string folder)
{
	if(!std::filesystem::is_directory(folder))
	{
		std::cout << "No folder '" << folder << "' found, exiting." << std::endl;
		exit(1);
	}
	int success1 = 0;
	std::vector<file_t> list = std::vector<file_t>();
	printf("FOLDER CONTENTS\n");
	int l = 0;
	for(const auto & entry : std::filesystem::directory_iterator(folder))
	{
		std::cout << l << ": "<<entry.path().string() << std::endl;
		file_t temp;
		temp.name = entry.path().string();
		temp.data = readFile(temp.name, success1);
		list.push_back( temp);
		if(!success1)
		{
			std::cout << "Error opening " << entry.path().string() << std::endl;
		}
		l++;
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
	std::vector<file_t> dir = folderContents(argv[1]);
	if(dir.empty())
	{
		fprintf(stderr, "empty folder, exiting \n");
		return 1;
	}
	lcss currentLongest = {
		0,std::vector<int>(),std::vector<std::pair<const std::string, int>>() ,(unsigned long) 0
	};
	int offset1 = 0, offset2 = 0;
	size_t key = 0;
	std::cout << "\n Working..." << std::endl;
	for(size_t i = 0; i < dir.size(); ++i)
	{	
		std::vector<int> *file1 = &(dir.at(i).data);
		for(size_t j = 0; j < i; ++j)// find if any new lcss are made by the addition of the new file
		{
			offset1 = 0;
			offset2 = 0;
			std::vector<int> *file2 = &(dir.at(i).data);

			std::vector<int> curSubString = longestCommonSubstring(file1, file2, offset1, offset2, key);
			// new longest
			if(curSubString.size() > currentLongest.length)
			{
				currentLongest.length = curSubString.size();
				currentLongest.key = key;
				currentLongest.substring = std::vector<int>(curSubString);
				currentLongest.where = std::vector<std::pair<const std::string, int>>();
				currentLongest.where.push_back(std::make_pair(dir.at(i).name, offset1));
				currentLongest.where.push_back(std::make_pair(dir.at(j).name, offset2));
			}
			// repeat longest
			if(( curSubString.size() == currentLongest.length) && (key == currentLongest.key))
			{
				currentLongest.where.push_back(std::make_pair(dir.at(j).name,offset2));
			}
		}
	}

	// std::cout << "lenght : " << currentLongest.length << " key : " << currentLongest.key << " # of files " << currentLongest.where.size() << std::endl;
	std::cout << " substring : ";
	for(size_t i = 0 ; i < currentLongest.substring.size(); ++i)
	{
		printBinary(currentLongest.substring.at(i)); //
		// std:: cout << toBinary(currentLongest.substring.at(i)) <<" ";
	}
	std::cout << " found at:" << std::endl;
	for(auto i : currentLongest.where)
	{
		std::cout << " " << i.first << " + " << i.second << std::endl;
	}
}