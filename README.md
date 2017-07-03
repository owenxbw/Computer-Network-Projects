#Computer Network Projects
1. Simple HTTP Client and Server

2. TCP-like Transport Protocol over UDP
•	Implemented sequence number, ack number and congestion control (TCP Reno) on UDP packets to achieve reliable data transfer between server and clients over unreliable link.

•	Server and clients were connected using UDP socket and implemented using C++.

•	Achieved robust file-transfer(>500MB) under high loss rate(>90%) and delay emulation.

•	Achieved robust multithreaded-server capable of transferring file to multiple clients.
