#pragma once

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

struct BaseMapperInternal {

		BaseMapperInternal();

		void emit(const string& key, const string& val);

		string filename;
		int n_output;
		hash<string> hasher;
};

inline BaseMapperInternal::BaseMapperInternal() {
// do nothing
}

inline void BaseMapperInternal::emit(const string& key, const string& val) {
	ofstream out;
	string temp = filename + to_string(hasher(key) % n_output)+".txt";
	out.open(temp,ios_base::app);
	out << key << ", " << val << endl;
	out.close();
}


/*-----------------------------------------------------------------------------------------------*/


struct BaseReducerInternal {

		BaseReducerInternal();

		void emit(const string& key, const string& val);

		string filename;
};

inline BaseReducerInternal::BaseReducerInternal() {
// do nothing
}

inline void BaseReducerInternal::emit(const string& key, const string& val) {
	//cout << "Dummy emit by BaseReducerInternal: " << key << ", " << val << endl;
	ofstream out;
	out.open(filename,ios_base::app);
	out << key << ", " << val << endl;
	out.close();
}
