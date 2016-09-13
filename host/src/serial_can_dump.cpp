#include "../include/serial_can_dump.hpp"



ExtendedCanFrame::ExtendedCanFrame(const uint32_t id, const uint8_t data_len, const uint8_t *data, const bool rtr)
:id(id),
data_len(data_len),
rtr(rtr)
{
	this->data = new uint8_t[data_len];
	for(size_t i=0; i<data_len; i++)
		this->data[i] = data[i];
}

ExtendedCanFrame::ExtendedCanFrame(const uint32_t id, const bool rtr)
:id(id),
rtr(rtr),
data_len(0),
data(new uint8_t[0])
{
}

ExtendedCanFrame::ExtendedCanFrame(const uint32_t id, const uint8_t data, const bool rtr)
:id(id),
rtr(rtr),
data_len(1),
data(new uint8_t[1])
{
	this->data[0] = data;
}

ExtendedCanFrame::ExtendedCanFrame(const uint32_t id, const uint16_t data, const bool rtr)
:id(id),
rtr(rtr),
data_len(1),
data(new uint8_t[2])
{
	this->data[0] = data >> 8;
	this->data[1] = data;
}

ExtendedCanFrame::~ExtendedCanFrame()
{
	delete[] data;
}

SerialCanDumpPort::SerialCanDumpPort(const std::string &device, unsigned int baud_rate)
:io(),
serial(io, device)
{
	serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
	startWaitingForPacket();
}

SerialCanDumpPort::~SerialCanDumpPort()
{
}

void SerialCanDumpPort::threadWorker()
{
	io.run();
}

ReceiveSignal& SerialCanDumpPort::onReceive()
{
	return _onReceive;
}

void SerialCanDumpPort::send(const ExtendedCanFrame& frame)
{
	uint8_t buf[HEADER_LEN + MAX_DATA_LEN];
	buf[0] = frame.id >> 24;
	buf[1] = frame.id >> 16;
	buf[2] = frame.id >>  8;
	buf[3] = frame.id >>  0;
	buf[4] = frame.data_len;
	for(size_t i = 0; i < frame.data_len; i++)
		buf[HEADER_LEN + i] = frame.data[i];
	boost::asio::async_write(
		serial,
		boost::asio::buffer(buf, HEADER_LEN + frame.data_len),
		[](const boost::system::error_code& error, const size_t transferred){}
	);
}

uint8_t* SerialCanDumpPort::getDataBuffer()
{
	return serialBuffer + HEADER_LEN;
}
void SerialCanDumpPort::startWaitingForPacket()
{
	boost::asio::async_read(
		serial,
		boost::asio::buffer(serialBuffer, HEADER_LEN),
		boost::bind(&SerialCanDumpPort::handleHeader, this, boost::asio::placeholders::error)
	);
}

void SerialCanDumpPort::handleHeader(const boost::system::error_code& error)
{
	if(!error)
	{
		boost::asio::async_read(
			serial,
			boost::asio::buffer(getDataBuffer(), serialBuffer[DATA_LEN_FIELD]),
			boost::bind(&SerialCanDumpPort::handleData, this, boost::asio::placeholders::error)
		);
	}
	else
		throw std::runtime_error("error while reading can header");
}

void SerialCanDumpPort::handleData(const boost::system::error_code& error)
{
	if (!error)
	{
		uint32_t tmp = ( (serialBuffer[0] << 24) | (serialBuffer[1] << 16) | (serialBuffer[2] << 8) | (serialBuffer[3]) );
		uint32_t id = tmp & ID_MASK;
		bool rtr = tmp & RTR_MASK;
		std::shared_ptr<ExtendedCanFrame> frame(new ExtendedCanFrame(id, serialBuffer[DATA_LEN_FIELD], getDataBuffer(), rtr));
		_onReceive(frame);
		startWaitingForPacket();
	}
	else
		throw std::runtime_error("error while reading can data");
}


