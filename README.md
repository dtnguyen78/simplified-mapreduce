### About MapReduce

MapReduce is a programming model and an associated implementation for processing and generating large data sets. Users specify a map function that processes a key/value pair to generate a set of intermediate key/value pairs, and a reduce function that merges all intermediate values associated with the same intermediate key. Programs written in this functional style are automatically parallelized and executed on a large cluster of commodity machines. The run-time system takes care of the details of partitioning the input data, scheduling the program’s execution across a set of machines, handling machine failures, and managing the required inter-machine communication.

### What's simplified here
1. **`The MapReduce library in the user program first splits the input files into M pieces of typically 16 megabytes to 64 megabytes (MB) per piece (controllable by the user via an optional parameter). It then starts up many copies of the program on a cluster of machines.`**
 - Instead of writing new split files, store the offsets from the original file as the file shards.
 - **Sharding**: Divide input into M number of shards, whereas M is number of file shards. They should be roughly of the same size specified in the config file. For example, if `map_kilobytes`(shard size) in the config file is 16 and you are given 3 input files of sizes 20kb, 15kb, 20kb then your **calculated** `number of shards M` should be `ceil(55/16) = 4`, and the shards will approximately look like:
         - shard 1 - ((file: file1, offsets: 0-16kb))
         - shard 2 - ((file: file1, offsets: 16-20kb), (file: file2, offsets: 0-12kb))
         - shard 3 - ((file: file2, offsets: 12-15kb), (file: file3, offsets: 0-13kb))
         - shard 4 - ((file: file3, offsets: 13-20kb))

 - The input shards to have complete record entries, for example, when the framework is running a word count program, then it should not be splitting in the middle of a word.
 - Instead of the map reduce library starting worker processes, they will manually start up before running the main binary.
 - Instead of running worker programs on different machines, it will run on a single machine by starting multiple instances of the worker program as processes on the same machine listening to different ports. 


2. **`One of the copies of the program is special – the master. The rest are workers that are assigned work by the master. There are M map tasks and R reduce tasks to assign. The master picks idle workers and assigns each one a map task or a reduce task.`**
 - Instead of running master as a different process, the framework makes a function call to the master to run and return back(already done for you in the code base).
 - Once M input shards are created, the master is supposed to assign each shard to one of the available workers.
 - Master can read the worker process addresses (ip:port) from the MapReduce specification structure provided by the framework.
 - Master will essentially be maintaining a worker pool, taking care of various things such as tracking how many workers are there in the system, what is the state of each worker: `AVAIALABLE, BUSY(doing map task/doing reduce task, etc.)`, when to assign what task to a worker, knowing when a worker is done.
 - `The communication of relevant instructions/data/results to/from the workers will be done through GRPC calls`.
 - For simplicity, you can `start your reduce phase when ALL of the map tasks are done`.
 
 
3. **`A worker who is assigned a map task reads the contents of the corresponding input split. It parses records out of the input data and passes each record to the user-defined Map function. The intermediate key/value pairs produced by the Map function are buffered in memory.`**
 - Application logic (Word count - counting the number of occurences for each word in the input, or finding mean-max temperature for each month from the given record of temperatures, etc.) should be written in BaseMapper's implementation.
 - Pass the output of the map function (key-value pairs) to the reducers by creating intermediate files on the disk. 


4. **`Periodically, the buffered pairs are written to local disk, partitioned into R regions by the partitioning function. The locations of these buffered pairs on the local disk are passed back to the master, who is responsible for forwarding these locations to the reduce workers.`**
 - Once the mappers have writen the output key, value pairs to the intermediate files, they need to pass the following information back to the master: `Indicate that the map task is done`, `Where the intermediate files are located`.
 
  
5. **`When a reduce worker is notified by the master about these locations, it uses remote procedure calls to read the buffered data from the local disks of the map workers. When a reduce worker has read all intermediate data, it sorts it by the intermediate keys so that all occurrences of the same key are grouped together. The sorting is needed because typically many different keys map to the same reduce task.`**
 - Theoretically, in real distributed environment, the intermediate files lie on local disks of mapper workers and reducers need to make remote file reads to get the data into their own local memory. However, for simplicity, the intermediate files are on the same file system as of your reducer worker, and hence you can read them through local file read system calls.
 - The final output is sorted on its keys.


6. **`The reduce worker iterates over the sorted intermediate data and for each unique intermediate key encountered, it passes the key and the corresponding set of intermediate values to the user’s Reduce function. The output of the Reduce function is appended to a final output file for this reduce partition.`**

7. **`When all map tasks and reduce tasks have been completed, the master wakes up the user program. At this point, the MapReduce call in the user program returns back to the user code.`**

