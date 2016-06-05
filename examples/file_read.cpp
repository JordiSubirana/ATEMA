#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

int main(int argc, char ** argv)
{
	try
	{
		File input_file;
		File output_file;
		string tmp;
		
		//Open input & output files (a file is created if it doesn't exist)
		input_file.open("txt/basic_file.txt");
		output_file.open("outputs/file_read_output.txt");
		
		//When opened, a file is pointing on the first line
		cout << "File current line : " << input_file.get_current_line_index() << endl;
		
		//Loop : read each line of input, checking its size then write it in output
		while (!input_file.end_of_file())
		{
			tmp = input_file.get_line();
			cout << tmp << " (size " << tmp.size() << ")" <<  endl;
			
			output_file.write(tmp);
			
			if (!input_file.end_of_file())
				output_file.write("\n");
		}
		
		cout << "File current line : " << input_file.get_current_line_index() << endl;
		
		//Checking for a specific line
		cout << "Specific line 7 (8th line) :" << endl;
		cout << input_file.get_line(7) << endl;
		
		cout << "File current line : " << input_file.get_current_line_index() << endl;
	}
	catch (const std::exception& e)
	{
		cout << "ERROR\n" << e.what() << endl;
	}
}