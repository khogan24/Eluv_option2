#include <iostream> 
#include <iomanip> 
#include <fstream> // io handling
#include <vector>
#include <string>
#include <utility> // pair
#include <bitset>


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
	int table[s1.size()][s2.size()];
	
	// any srting of length 0 has no commonn substring with any other strings, so set those indicies to 0
	for(int i = 0; i < s1.size(); ++i)
	{
		table[i][0] = 0;
	}
	for(int i = 0; i < s2.size(); ++i)
	{
		table[0][i] = 0;
	}

	int s1_offset = 0, s1_temp = 0, s2_offset = 0, s2_temp = 0, len = 0, len_temp = 0;
	
	// loop through both string comapring elements
	for(int s1_index = 0; s1_index < s1.size(); ++s1_index)
	{
		for(int s2_index = 0; s2_index < s2.size(); ++s2_index)
		{
			// a substring is only common if both strings contain the same substring of chars
			// if any are not the same, the substring ends
			if(s1[s1_index] == s2[s2_index])
			{
				// increment length of sub-string
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
		std::cout<< "No file " << path_to_file << " found, skipping..." << std::endl;
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


int main()
{

	// TODO add flags? maybe
	int error = 0;
	std::string s = "sample. ";
	std::vector<std::string> fileNames = std::vector<std::string>(10); // list of filenmes
	std::vector<std::vector<bool>> fileData = std::vector<std::vector<bool>>(fileNames.size()); // list of data in each file
	lcss currentLongest = {
							0,
							std::vector<bool>(),
							std::vector<std::pair<const char*, int>>(fileNames.size()),
							0
	};
	for(int i = 0; i < fileNames.size(); ++i)
	{	
		s.at(s.length()-1) = i;
		fileData.push_back(readFile(s.c_str(), error));
		if(error)
		{
			//TDOD handle this better
			std::cout<< "error encountered " << std::endl;
			return 0;
		}
		for(int j = 0; j < i -1 ; ++j)// find if any new lcss are made by the addition of the new file
		{	int offset_i = 0, offset_j = 0;
			size_t key = 0;
			std::vector<bool> temp = longestCommonSubstring(fileData.at(i), fileData.at(j),offset_i, offset_j, key );
			if(temp.size() > currentLongest.length)
			{
				currentLongest.length = temp.size();
				currentLongest.substring = std::vector<bool>(temp);
				currentLongest.where = std::vector<std::pair<const char*,int>>();
				s.at(s.length()-1) = i;
				currentLongest.where.push_back(std::make_pair(s.c_str(), offset_i));
				s.at(s.length()-1) = j;
				currentLongest.where.push_back(std::make_pair(s.c_str(), offset_j));
				currentLongest.key = key;
			}
			if(temp.size() == currentLongest.length && key == currentLongest.key)
			{
				s.at(s.length()-1) = j;
				currentLongest.where.push_back(std::make_pair(s.c_str(), offset_j));
			}
		}
	}
}