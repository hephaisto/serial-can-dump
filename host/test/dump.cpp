#include <iostream>
#include <iomanip>
using std::cout;

#include <string>
using std::string;

#include <serial_can_dump.hpp>

void printPacket(std::shared_ptr<const ExtendedCanFrame> frame)
{
	std::cout << std::setfill('0') << std::setw(8) << std::hex << frame->id;
	std::cout << "   " << std::setw(1) << (int)(frame->data_len);
	std::cout << "  ";
	for(size_t i=0; i<frame->data_len; i++)
		std::cout << " " << std::setw(2) << std::hex << (int)(frame->data[i]);
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cout<<"USAGE: " << argv[0] << " device baudrate\n";
		return 1;
	}
	string device(argv[1]);
	string baud(argv[2]);
	boost::asio::io_service io;
	SerialCanDumpPort port(io, device, std::stoul(baud));
	port.onReceive().connect(printPacket);
	std::cout << "ID       |len| data\n";
	port.threadWorker(io);
	return 0;
}
