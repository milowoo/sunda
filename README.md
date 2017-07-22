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

   业余时间研究了oracle tuxedo中间件系统，在保证与tuxedo接口规范兼容条件下，使用全新架构实现了一款高性能、高并发的分布式交易中间件--SUNDA 
   涉及到的技术：epoll异步非堵塞通讯，IPC进程间通讯(共享内存，消息队列,信号)，进程池,线程池等. 
   SUNDA产品特点： 
   1:支持集群部署,系统方便扩容 
   2:支持高并发请求.通讯接入采用典型的HAHS设计模式.通过epoll非堵塞通讯和线程池技术支持高并发的链接请求和读写;业务处理采用进程池并发方式,并且通讯和业务处理分开.在数据流程上也极度的精简，采用最佳的设计达到支持高并发处理的需求 
   3:支持负载均衡，采用MSSQ等技术，合理处理请求． 
   4:灵活易扩展性:业务处理服务采用插件的方式设计.非常方便的进行动态增加或者修改.而且对于异常的服务系统能自动的屏蔽. 
   5:完善的灾备机制,对防止系统雪崩上进行特殊的防护. 6:开放兼容性:支持客户端的同步短链接，异步长链接等各种TCP/IP网络通讯．
