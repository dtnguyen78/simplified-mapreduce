### MapReduce Implementation
MapReduce is a programming model and an associated implementation for processing and generating large data sets. Users specify a map function that processes a key/value pair to generate a set of intermediate key/value pairs, and a reduce function that merges all intermediate values associated with the same intermediate key. Programs written in this functional style are automatically parallelized and executed on a large cluster of commodity machines. The run-time system takes care of the details of partitioning the input data, scheduling the program’s execution across a set of machines, handling machine failures, and managing the required inter-machine communication.

### src directory
1. **`masterworker.proto`** - contains grpc protocal between master and worker programs.

2. **`file_shard.h`** - contains data structure for handling file splits. This is what the master passes to the workers when the Map function is used. The framework will also call other functions in this file to generate file shards if given the split size.

3. **`mapreduce_spec.h`** - Data structure populated by framework. Framework will also validate the contents of the structure (input file paths, number of workers, number of output files, etc.). This ultimately gets passed to the master.

4. **`master.h`** - does the book keeping and communicates with the workers.

5. **`mr_tasks.h`** - handles a map/reduce task. The user's map/reduce function will use the emit function from this file to write intermediate key/value papers.

6. **`worker.h`** - this guides workers in running map and reduce tasks upon receiving a request from master.

### How to run
1. Run `make` in /test and /src
2. Make sure /src/output is empty
3. In /test, start multiple worker processes (`./mr_worker localhost:50051 & ./mr_worker localhost:50052 & ./mr_worker localhost:50053 & ./mr_worker localhost:50054 & ./mr_worker localhost:50055 & ./mr_worker localhost:50056`)
4. In /test in a separate terminal, start MapReduce (`./mrdemo config.ini`)
5. Once MapReduce completes, run `killall mr_worker` to kill all worker processes
6. Check /test/output for resulting files - it should contain the file shards and the output files from the reduce function
7. Run `make clean`
