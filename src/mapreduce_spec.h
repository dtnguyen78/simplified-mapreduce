#pragma once

#include <string>
#include <cassert>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

struct MapReduceSpec {
	int n_workers;
	int n_output_files;
	int map_kilobytes;
	vector<string> worker_ipaddr_ports;
	vector<string> input_files;
	string output_dir;
	string user_id;
};

inline bool read_mr_spec_from_config_file(const string& config_filename, MapReduceSpec& mr_spec) {
	ifstream file(config_filename);
	if (file.is_open()) {
		string line;
		while (getline(file,line)) {
			if (line.find("n_workers") != string::npos) {
				mr_spec.n_workers = stoi(line.substr(10,line.size()));
			} else if (line.find("worker_ipaddr") != string::npos) {
				stringstream ss(line.substr(20,line.size()));
				while (ss.good()) {
					string addr;
					getline(ss,addr,',');
					mr_spec.worker_ipaddr_ports.push_back(addr);
				}
				ss.clear();
			} else if (line.find("input_files") != string::npos) {
				stringstream ss(line.substr(12,line.size()));
				while (ss.good()) {
					string input;
					getline(ss,input,',');
					mr_spec.input_files.push_back(input);
				}
				ss.clear();
			} else if (line.find("output_dir") != string::npos) {
				mr_spec.output_dir = line.substr(11,line.size());
			} else if (line.find("n_output") != string::npos) {
				mr_spec.n_output_files = stoi(line.substr(15,line.size()));
			} else if (line.find("map_kilo") != string::npos) {
				mr_spec.map_kilobytes = stoi(line.substr(14,line.size()));
			} else if (line.find("user_id") != string::npos) {
				mr_spec.user_id = line.substr(8,line.size());
			}
		}
		file.close();
	} else {
		cerr << "Cannot access " << config_filename << endl;
	}

	return true;
}

inline void printConfig(const MapReduceSpec& mr_spec){
}

/* CS6210_TASK: validate the specification read from the config file */
inline bool validate_mr_spec(const MapReduceSpec& mr_spec) {
	assert(mr_spec.n_workers > 0 && mr_spec.n_output_files > 0 && mr_spec.map_kilobytes > 0);
	assert(mr_spec.n_workers == mr_spec.worker_ipaddr_ports.size());

	ifstream file(mr_spec.output_dir);
	assert(file.is_open());
	file.close();

	for (auto &input : mr_spec.input_files) {
		ifstream file(input);
		assert(file.is_open());
		file.close();
	}

	cout << "Verified: Specification in config file is valid" << endl;

	return true;
}
