# sunda
In the spare time, I studied the oracl tuxedo middleware system, and used the new architecture to realize a high-performance and highly concurrent distributed transaction middleware under the condition of ensuring the compatibility with the tuxedo interface specification.
SUNDA relates to the technology: epoll asynchronous, non blocking communication, IPC interprocess communication (shared memory, message queues, signals), process pools, thread pools, and so on.
SUNDA product features:
1:Support cluster deployment, easy expansion of the system.
2:Supports high concurrent requests. Communication access uses a typical HAHS design pattern;through epoll non blocking communications and thread pool technology to support highly concurrent link requests and read and write;business processes use process pooling concurrency;And communication is separate from business processing.Extremely streamlined in the data flow,the best design is used to support the requirement of high concurrency processing.
3:Support load balancing, use MSSQ and other technologies to handle requests reasonably.
4:Flexible and extensible: business process services are designed using plug-ins, very convenient for dynamic addition or modification, and can be automatically blocked for exceptional service systems.
5:The disaster recovery mechanism, to prevent the system of special avalanche protection.
6:Open compatibility: supports client synchronization, short links, asynchronous long links and other TCP/IP network communications.
