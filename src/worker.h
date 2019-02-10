#pragma once

#include <mr_task_factory.h>
#include "mr_tasks.h"
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
#include "masterworker.grpc.pb.h"
#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using masterworker::MasterWorker;
using masterworker::WorkerReply;
using masterworker::WorkerRequest;
using namespace std;

extern shared_ptr<BaseMapper> get_mapper_from_task_factory(const string& user_id);
extern shared_ptr<BaseReducer> get_reducer_from_task_factory(const string& user_id);

/* CS6210_TASK: Handle all the task a Worker is supposed to do.
	This is a big task for this project, will test your understanding of map reduce */
class Worker {
public:
	friend class BaseMapper;
	friend class BaseReducer;

	/* DON'T change the function signature of this constructor */
	Worker(string ip_addr_port);
	~Worker() {
		server_->Shutdown();
		cq_->Shutdown();
	}

	/* DON'T change this function's signature */
	bool run();

private:
	/* NOW you can add below, data members and member functions as per the need of your implementation*/
	class CallData  {
		public:
		CallData(MasterWorker::AsyncService* service, ServerCompletionQueue* cq)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
			Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestassignMapReduce(&ctx_, &request_, &responder_, cq_, cq_,this);
			} else if (status_ == PROCESS) {
				new CallData(service_, cq_);
				string output = request_.output_dir() + "/";
				string uid = request_.user_id();

				// Actual MapReduce processing
				if(request_.do_map()) {
					cout << "Received: MAP TASK" << endl;
					Map(output,uid); // use Map helper function
					cout << "Finished: MAP TASK" << endl;
				} else {
					cout << "Reducing..." << endl;
					Reduce(output,uid); // use Reduce helper function
					cout << "Finished Reducing." << endl;
				}

				status_ = FINISH;
				responder_.Finish(reply_, Status::OK, this);
			} else {
				
				GPR_ASSERT(status_ == FINISH);
				delete this;
			}
		}

		private:
		void Map(const string output, const string uid) {
			string file_shard = request_.map();
			int n_files = request_.n_output();
			string tmp = file_shard.substr(0, file_shard.find(".txt"));

			auto mapper = get_mapper_from_task_factory(uid);
			mapper->impl_->filename = output + tmp + "_";
			mapper->impl_->n_output = n_files;

			ifstream file(output + file_shard);
			string line;
			while(getline(file,line)) {
				mapper->map(line);
			}
			file.close();

			for (int i = 0; i < n_files; i++) {
				reply_.add_map(output + tmp +"_"+to_string(i)+".txt");
			}
		}

		void Reduce(const string output, const string uid) {
			vector<string> map_files;
			for(int i=0; i<request_.reduce_size();i++) {
				map_files.push_back(request_.reduce(i));
			}

			map<string,vector<string>> map_;
			for (auto &mf : map_files) {
				ifstream file(mf.c_str());
				if(file.is_open()) {
					string line;
					while(getline(file,line)) {
						string line2 = line.substr(0, line.find(","));
						map_[line2];
						if (map_.find(line2) == map_.end() ) {
							map_[line2].push_back("1");
						} else {
							map_[line2].push_back("1");
						}
					}
					file.close();
				} else {
					cerr << "Unable to access " << mf.c_str() << endl;
				}
			}

			auto reducer = get_reducer_from_task_factory(uid);
			string reduce_file = output + "out_" + map_files[0].substr(map_files[0].find_last_of("_")+1);
			reducer->impl_->filename = reduce_file;
			reply_.set_reduce(reduce_file);
			for (auto it = map_.begin(); it != map_.end(); it++) {
				reducer->reduce(it->first,it->second);
			}
		}

		MasterWorker::AsyncService* service_;
		ServerCompletionQueue* cq_;
		ServerContext ctx_;
		WorkerRequest request_;
		WorkerReply reply_;
		ServerAsyncResponseWriter<WorkerReply> responder_;
		enum CallStatus { CREATE, PROCESS, FINISH };
		CallStatus status_;
	};

	void HandleRpcs() {
		new CallData(&service_, cq_.get());
		void* tag;
		bool ok;
		while (true) {
			GPR_ASSERT(cq_->Next(&tag, &ok));
			GPR_ASSERT(ok);
			static_cast<CallData*>(tag)->Proceed();
		}
	}

	unique_ptr<ServerCompletionQueue> cq_;
	MasterWorker::AsyncService service_;
	unique_ptr<Server> server_;
};

/* CS6210_TASK: ip_addr_port is the only information you get when started.
	You can populate your other class data members here if you want */
Worker::Worker(string ip_addr_port) {
	ServerBuilder builder;
	builder.AddListeningPort(ip_addr_port, grpc::InsecureServerCredentials());
	builder.RegisterService(&service_);
	cq_ = builder.AddCompletionQueue();
	server_ = builder.BuildAndStart();
	cout << "Server listening on " << ip_addr_port << endl;
}

/* CS6210_TASK: Here you go. once this function is called your woker's job is to keep looking for new tasks 
	from Master, complete when given one and again keep looking for the next one.
	Note that you have the access to BaseMapper's member BaseMapperInternal impl_ and 
	BaseReduer's member BaseReducerInternal impl_ directly, 
	so you can manipulate them however you want when running map/reduce tasks*/
bool Worker::run() {
	HandleRpcs();
	return true;
}
