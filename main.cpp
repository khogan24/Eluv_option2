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
struct lcss
{
	std::vector<bool> substring;
	std::vector<std::pair<char*, int>> where;

};



/*
	finds and returns the longest common substring between 2 lists
	sets offset1 and offset2 to the respective locations of the first charachter in the strings
	an empty vector and both at 0 means no such string was found
		the only way for a binary file to do this is if one is all 1 and the other is all 0
	if 2 substring are of equal lenght, the first one found is returned
*/
std::vector<bool> longestCommonSubstring(const  std::vector<bool> & s1, const std::vector<bool> & s2, int & offset1, int & offset2)
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
				// upon reaching a new substring, record the offsets
				if(s1_temp == 0) s1_temp = s1_index;
				if(s2_temp == 0) s1_temp = s2_index;
				len_temp++;
				table[s1_index][s2_index] = 1;
			}
			else
			{
				table[s1_index][s2_index] = 0;
				if(len_temp > len)
				{
					len = len_temp;
				}
				s1_offset = MAX(s1_offset,s1_temp);
				s2_offset = MAX(s2_offset,s2_temp);
			}
		}
	}

}

/*
	Opens and reads binary file at path_to_file 
	returns contents in a vector<bool>
	returns empty vector if no such file is found
	sets the success value to 1 if read properly, and 0 on error
*/
std::vector<bool> readFile( char* path_to_file, int& success )
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
	char* s = "sample.1";
	std::vector<std::string> fileList = std::vector<std::string>(10);
	for(int i = 0; i < fileList.size(); ++i)
	{	
		readFile(s, error);
		if(error)
		{
			//TDOD handle this better
			std::cout<< "error encountered " << std::endl;
			return 0;
		}
	}
}