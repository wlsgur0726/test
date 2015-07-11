#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#endif
#include <stdio.h>
#include <thread>
#include "../../asd/asd/include/socket.h"

//const char RecverAddr[] = "192.168.0.4"; uint16_t RecverPort = 54321;
const char RecverAddr[] = "192.168.135.128"; uint16_t RecverPort = 54321;

int main(int argc, char* argv[]) 
{
	try {
		//asd::IpAddress addr("192.168.0.4", 10101);
		asd::IpAddress addr("fe80::6dcf:ba57:e2db:6bc0%10", 10101);
		asd::EasySocket sock(addr.GetAddressFamily());
		asd::puts(addr.ToString());

		sock.Bind(addr);
		sock.Listen();
		
		std::thread t([&](){
			try {
				asd::EasySocket consock;
				consock.Connect(addr);
			}
			catch (std::exception& e) {
				puts(e.what());
			}
		});

		asd::EasySocket acceptedSock;
		asd::IpAddress acceptedAddr;
		sock.Accept(*acceptedSock.m_socket, acceptedAddr);
		t.join();

		asd::puts(acceptedAddr.ToString());

		asd::IpAddress out;
		acceptedSock.m_socket->GetSockName(out);
		asd::puts(out.ToString().GetData());

		acceptedSock.m_socket->GetPeerName(out);
		asd::puts(out.ToString().GetData());
	}
	catch (std::exception& e) {
		puts(e.what());
	}

	return 0;
}
