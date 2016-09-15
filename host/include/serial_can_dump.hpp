#include <memory>
#include <string>

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
namespace bs2 = boost::signals2;


class ExtendedCanFrame
{
public:
	ExtendedCanFrame(const uint32_t id, const uint8_t data_len, const uint8_t *data, const bool rtr=false);
	ExtendedCanFrame(const uint32_t id, const bool rtr=false);
	ExtendedCanFrame(const uint32_t id, const uint8_t data, const bool rtr=false);
	ExtendedCanFrame(const uint32_t id, const int8_t data, const bool rtr=false);
	ExtendedCanFrame(const uint32_t id, const uint16_t data, const bool rtr=false);
	ExtendedCanFrame(const uint32_t id, const int16_t data, const bool rtr=false);

	~ExtendedCanFrame();
	uint32_t id;
	uint8_t data_len;
	uint8_t *data;
	bool rtr;
};

typedef bs2::signal<void (std::shared_ptr<const ExtendedCanFrame>)> ReceiveSignal;

class SerialCanDumpPort
{
public:
	SerialCanDumpPort(const std::string &device, unsigned int baud_rate);
	~SerialCanDumpPort();
	ReceiveSignal& onReceive();
	void send(const ExtendedCanFrame& frame);
	void startWaitingForPacket();
	void threadWorker();
private:
	static const size_t HEADER_LEN = 5;
	static const size_t MAX_DATA_LEN = 16;
	static const size_t DATA_LEN_FIELD = 4;
	static const uint32_t EXT_MASK = (1 << (5+24) );
	static const uint32_t RTR_MASK = (1 << (6+24) );
	static const uint32_t ID_MASK = ~( RTR_MASK | EXT_MASK );

	ReceiveSignal _onReceive;
	boost::asio::io_service io;
	boost::asio::serial_port serial;

	uint8_t* getDataBuffer();
	void handleHeader(const boost::system::error_code& error);
	void handleData(const boost::system::error_code& error);

	uint8_t serialBuffer[HEADER_LEN + MAX_DATA_LEN];
};

