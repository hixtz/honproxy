#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#define NULL 0

namespace udpProxy
{
	namespace ip = boost::asio::ip;
	using namespace boost::asio;
	using namespace std;
	const int max_data_length = 1024 * 10;

	//constants 
	const int SRVR_PORT = 11235;
	const int LISTEN_PORT = 12235;
  ip::address_v4 s_remoteIp;
  USHORT s_remotePort = NULL;
  ip::address_v4 s_clientIp;
  USHORT s_clientPort = NULL;

  string FREE_SRVR_REPORT = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x6a\x3d\x95\xa2\x03\x00\x00\x00\x1c\x2a\x88\x6b\x2d\x88\x80\x21\x5c\xb0\xb0\x18\x1f\x2f\xaf\xcd\x76\xc9\xfa\x33\x00\x00\x01\x66\x3e\x78\x55\x6e\x6e\x61\x6d\x65\x64\x5f\x53\x65\x72\x76\x65\x72\x00\x00\x00\x00\x34\x2e\x31\x30\x2e\x31\x2e\x30\x00\x00\x01\x00\x00\x00\x00\x6e\x6f\x72\x6d\x61\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	class m_udpProxyServer
	{				
	private:		
		ip::udp::socket listener_socket;
	  ip::address_v4 _remotehost;
		int _remoteport;
		ip::address_v4 _clientip;
		int _clientport;
		ip::udp::endpoint downstream_remoteUdpEndpoint;
		
		unsigned char downstream_data_[max_data_length];
		unsigned char upstream_data_[max_data_length];

	public:
		m_udpProxyServer(io_service& io):
		listener_socket(io,ip::udp::endpoint(ip::udp::v4(),LISTEN_PORT)),
			_remotehost(s_remoteIp),
			_remoteport(s_remotePort),
		  _clientip(s_clientIp),
			_clientport(s_clientPort)
		{
			StartListener();
		}

		void StartListener()
		{
      try
      {
        while (true)
        {
		      std::cerr << "starting listener" << std::endl;

			    listener_socket.receive_from(boost::asio::buffer(downstream_data_,max_data_length),downstream_remoteUdpEndpoint);

					s_remoteIp = downstream_remoteUdpEndpoint.address().to_v4();
					s_remotePort = downstream_remoteUdpEndpoint.port();

					if (downstream_remoteUdpEndpoint.port() == SRVR_PORT)
          {
            ForwardToClient(ip::udp::endpoint(s_clientIp, s_clientPort));
          }

					if (downstream_remoteUdpEndpoint.port() != SRVR_PORT)
          {
            s_clientIp = s_remoteIp;
            s_clientPort = s_remotePort;
						
            ForwardToServer();

          }	
        }
      }
      catch (...)
      {
      }



		}

		void ForwardToClient(ip::udp::endpoint downstream_remoteUdpEndpoint)
    {
			listener_socket.send_to(downstream_data_, downstream_remoteUdpEndpoint);
    }

		void ForwardToServer()
		{

		  listener_socket.send_to(downstream_data_,downstream_remoteUdpEndpoint);
		}

	};
}


int main()
{
		std::cerr << "usage: udpProxyServer<local host ip> <local port> <forward host ip> <forward port>" << std::endl;
		

	boost::asio::io_service ios;

	try
	{
		udpProxy::m_udpProxyServer svrTest(ios);

		ios.run();
	}
	catch(std::exception& e)
	{
		std::cerr << "main : Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
// LINK : -L/usr/lib -lstdc++ -l pthread -l boost_thread-mt -l boost_system