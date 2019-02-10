#pragma once

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

/* CS6210_TASK Implement this data structureas per your implementation.
		You will need this when your worker is running the map task*/
struct BaseMapperInternal {

		/* DON'T change this function's signature */
		BaseMapperInternal();

		/* DON'T change this function's signature */
		void emit(const string& key, const string& val);

		/* NOW you can add below, data members and member functions as per the need of your implementation*/
		string filename;
		int n_output;
		hash<string> hasher;
};


/* CS6210_TASK Implement this function */
inline BaseMapperInternal::BaseMapperInternal() {
// do nothing
}

/* CS6210_TASK Implement this function */
inline void BaseMapperInternal::emit(const string& key, const string& val) {
	ofstream out;
	string temp = filename + to_string(hasher(key) % n_output)+".txt";
	out.open(temp,ios_base::app);
	out << key << ", " << val << endl;
	out.close();
}


/*-----------------------------------------------------------------------------------------------*/


/* CS6210_TASK Implement this data structureas per your implementation.
		You will need this when your worker is running the reduce task*/
struct BaseReducerInternal {

		/* DON'T change this function's signature */
		BaseReducerInternal();

		/* DON'T change this function's signature */
		void emit(const string& key, const string& val);

		/* NOW you can add below, data members and member functions as per the need of your implementation*/
		string filename;
};

/* CS6210_TASK Implement this function */
inline BaseReducerInternal::BaseReducerInternal() {
// do nothing
}

/* CS6210_TASK Implement this function */
inline void BaseReducerInternal::emit(const string& key, const string& val) {
	//cout << "Dummy emit by BaseReducerInternal: " << key << ", " << val << endl;
	ofstream out;
	out.open(filename,ios_base::app);
	out << key << ", " << val << endl;
	out.close();
}
