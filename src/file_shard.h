#pragma once

#include <vector>
#include "mapreduce_spec.h"
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

/* CS6210_TASK: Create your own data structure here, where you can hold information about file splits,
     that your master would use for its own bookkeeping and to convey the tasks to the workers for mapping */
struct FileShard {
	string filename;
};

/* CS6210_TASK: Create fileshards from the list of input files, map_kilobytes etc. using mr_spec you populated  */ 
inline bool shard_files(const MapReduceSpec& mr_spec, vector<FileShard>& fileShards) {
	string line;
	int curr_size = 0;
	int numberInput = mr_spec.input_files.size();

	FileShard fs;
	fs.filename = "fileshard" + to_string(0) + ".txt";
	fileShards.push_back(fs);

	ofstream out(mr_spec.output_dir + "/fileshard" + to_string(0) + ".txt");
	for(int i = 0; i < mr_spec.input_files.size(); i++) {
		ifstream file(mr_spec.input_files[i]);
		while(getline(file,line)) {
			out << line << endl;
			curr_size += line.size(); // includes '\n'
			if(curr_size > mr_spec.map_kilobytes * 1024) {
				out.close();
				out.open(mr_spec.output_dir + "/fileshard" + to_string(i) + ".txt");
				fs.filename = "fileshard" + to_string(i) + ".txt";
				fileShards.push_back(fs);
				curr_size = 0; // reset curr_size for the next FileShard
			}
		}
		file.close();
	}
	out.close();

	return true;
}
