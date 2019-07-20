#pragma once

#include "mapreduce_spec.h"
#include "file_shard.h"
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
#include "masterworker.grpc.pb.h"
#include <string>
#include <cstdio>

using masterworker::MasterWorker;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;
using masterworker::WorkerRequest;
using masterworker::WorkerReply;
using namespace std;

class Master {

	public:
	Master(const MapReduceSpec&, const vector<FileShard>&);

	bool run();

	private:
	int n_fileshards;
	MapReduceSpec mr_spec_;		// local copy of given mr_spec
	vector<FileShard> file_shards_; // local copy of given file_shards
	vector<WorkerReply> map_files;
	vector<WorkerReply> reduce_files;
	vector<unique_ptr<MasterWorker::Stub>> stubs;
	vector<string> mapped_files;

	// Map function
	void assignMap() {
		cout << "Assigned MAP task" << endl;

		vector<WorkerRequest> request(n_fileshards);
		vector<WorkerReply> reply(n_fileshards);
		vector<ClientContext> context(n_fileshards);
		CompletionQueue cq;
		vector<Status> status(n_fileshards);
		vector<unique_ptr<grpc::ClientAsyncResponseReader<WorkerReply>>> rpc(n_fileshards);

		int index = 0;
		for(int i = 0; i < n_fileshards; i++) {
			// Set up request message for Map
			request[i].set_do_map(true);
			request[i].set_map(file_shards_[i].filename);
			request[i].set_n_output(mr_spec_.n_output_files);
			request[i].set_output_dir(mr_spec_.output_dir);
			request[i].set_user_id(mr_spec_.user_id);

			rpc[i] = stubs[index]->AsyncassignMapReduce(&context[i],request[i],&cq);
			rpc[i]->Finish(&reply[i],&status[i],&file_shards_[i].filename);
			index = (index + 1) % mr_spec_.n_workers;
		}

		//FIXME: what to do when a worker thread fails?

		void* got_tag;
		bool ok = false;
		int doneCount = 0;
		while(doneCount < n_fileshards) {
			GPR_ASSERT(cq.Next(&got_tag, &ok));
			GPR_ASSERT(ok);
			for(int i = 0; i < n_fileshards; i++) {
				if(got_tag == &file_shards_[i]) {
					if(status[i].ok()) {
						map_files.push_back(reply[i]);
						doneCount++;
						break;
					} else {
						cerr << "Unable to do RPC for " << file_shards_[i].filename << endl;
					}						
				}
			}
		}
	} //end of Map function

	//Reduce function
	void assignReduce() {
		cout << "Assigned REDUCE task" << endl;

		vector<WorkerRequest> request(mr_spec_.n_output_files);
		vector<WorkerReply> reply(mr_spec_.n_output_files);
		vector<ClientContext> context(mr_spec_.n_output_files);
		CompletionQueue cq;
		vector<Status> status(mr_spec_.n_output_files);
		vector<unique_ptr<grpc::ClientAsyncResponseReader<WorkerReply>>> rpc(mr_spec_.n_output_files);

		// fill in mapped_files vector with map files from reply message
		for (int i = 0; i < mr_spec_.n_output_files; i++) {
			for(int j = 0; j < n_fileshards; j++) {
				mapped_files[i] = map_files[j].map(i);
			}
		}

		int index = 0;
		for(int i = 0; i < mr_spec_.n_output_files; i++) {
			// Set up request message for Reduce
			request[i].set_do_map(false);
			request[i].add_reduce(mapped_files[i]);
			request[i].set_n_output(mr_spec_.n_output_files);
			request[i].set_output_dir(mr_spec_.output_dir);
			request[i].set_user_id(mr_spec_.user_id);

			rpc[i] = stubs[index]->AsyncassignMapReduce(&context[i],request[i],&cq);
			rpc[i]->Finish(&reply[i],&status[i],&mapped_files[i]);
			index = (index + 1) % mr_spec_.n_workers;
		}

		void* got_tag;
		bool ok = false;
		int doneCount = 0;
		while(doneCount < mr_spec_.n_output_files) {
			GPR_ASSERT(cq.Next(&got_tag, &ok));
			GPR_ASSERT(ok);
			for(int i = 0; i < mr_spec_.n_output_files; i++) {
				if(got_tag == &mapped_files[i]) {
					if(status[i].ok()) {
						reduce_files.push_back(reply[i]);
						doneCount++;
						break;
					} else {
						cerr << "Unable to do RPC for " << file_shards_[i].filename << endl;
					}						
				}
			}
		}

		cout << "Cleaning up intermediate files..." << endl;

	} //end of Reduce function
};


Master::Master(const MapReduceSpec& mr_spec, const std::vector<FileShard>& file_shards) {
	mr_spec_ = mr_spec;
	n_fileshards = file_shards.size();
	for (auto fs : file_shards) {
		file_shards_.push_back(fs);
	}

	stubs.resize(mr_spec_.n_workers + 1);
	mapped_files.resize(mr_spec.n_output_files + 1);

	for (int i = 0; i < mr_spec_.n_workers; i++) {
		shared_ptr<grpc::Channel> channel = grpc::CreateChannel(mr_spec_.worker_ipaddr_ports[i], grpc::InsecureChannelCredentials());
		stubs[i] = MasterWorker::NewStub(channel);
	}
}

bool Master::run() {
	assignMap();
	assignReduce();
	cout << "Time to wakeup!" << endl;

	return true;
}
