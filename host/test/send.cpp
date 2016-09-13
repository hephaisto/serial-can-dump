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
	if((argc < 4) || (argc > 4+16))
	{
		std::cout<<"USAGE: " << argv[0] << " device baudrate can_id [data [data ...]]\n";
		return 1;
	}
	string device(argv[1]);
	string baud(argv[2]);
	string can_id(argv[3]);
	uint8_t data_len = argc-4;
	uint8_t *data = new uint8_t[data_len];
	for(uint8_t i=0; i<data_len; i++)
		data[i] = std::stoul(argv[4+i]);

	SerialCanDumpPort port(device, std::stoul(baud));
	port.send(ExtendedCanFrame(std::stoul(can_id), data_len, data));
	delete[] data;
	return 0;
}
