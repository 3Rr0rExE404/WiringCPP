#include "wiringCPP.hpp"

int8_t WiringCPP::pinMode(uint8_t chip_id, uint8_t pin_id, uint8_t mode)
{
	try
	{
		struct gpiod_chip *chip = gpiod_chip_open_by_number(chip_id);
		if(!chip) return -2;
		
		struct gpiod_line *pin = gpiod_chip_get_line(chip, pin_id);
		if(!pin) return -3;
		
		if(!mode)	//0 = input
		{
			if(gpiod_line_request_input(pin, "wiringC++")) return -4;
		}
		else	//1-255 = output
		{
			if(gpiod_line_request_output(pin, "wiringC++", 0)) return -5;
		}
		WiringCPP::pinmap[(uint16_t)((chip_id << 8) | pin_id)] = pin;

		return 0;
	}
	catch(...)
	{
		return -1;
	}
}

int8_t WiringCPP::digitalRead(uint8_t chip_id, uint8_t pin_id)
{
	try
	{
		int8_t val = gpiod_line_get_value(WiringCPP::pinmap.at((uint16_t)((chip_id << 8) | pin_id)));
		if(val == -1) throw std::runtime_error("digitalRead failed!");
		
		return val;
	}
	catch(...)
	{
		throw std::runtime_error("digitalRed failed!"); //only chuck norris can reach here :)
	}
}

int8_t WiringCPP::digitalWrite(uint8_t chip_id, uint8_t pin_id, uint8_t state)
{
	try
	{
		if(gpiod_line_set_value(WiringCPP::pinmap.at((uint16_t)((chip_id << 8) | pin_id)), !!state)) return -2;
		
		return 0;
	}
	catch(...)
	{
		return -1; //pin not initialized in most cases (out_of_range exception)
	}
}

int8_t WiringCPP::pinClose(uint8_t chip_id, uint8_t pin_id)
{
	try
	{
		gpiod_line_release(WiringCPP::pinmap.at((uint16_t)((chip_id << 8) | pin_id)));
		return 0;
	}
	catch(const std::exception& e)
	{
		return -1;
	}
	
}

int8_t WiringCPP::wiringCPPSPISetupMode(uint8_t bus, uint8_t channel, uint32_t speed, uint8_t mode)
{
	try
	{
		WiringCPP::spidev sd;
		
		sd.name = "/dev/spidev" + std::to_string(bus) + '.' + std::to_string(channel);
		if((sd.fd = open(sd.name.c_str(), O_RDWR)) < 0) return -2; // if spidev open failed
		
		mode &= 3; //allow only 0, 1, 2, 3
		if(ioctl(sd.fd, SPI_IOC_WR_MODE, &mode) < 0) return -3; // mode change failed
		sd.bpw = 8;
		if(ioctl(sd.fd, SPI_IOC_WR_BITS_PER_WORD, &sd.bpw) < 0) return -4; // bits per word change failed
		if(ioctl(sd.fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) return -5; // max speed change failed
		sd.speed = speed;
		sd.delay = 0;
		
		WiringCPP::spidevmap[(uint16_t)((bus << 8) | channel)] = sd; //add spidev to map
		return 0;
	}
	catch(...)
	{
		return -1;
	}
}

int8_t WiringCPP::wiringCPPSPISetup(uint8_t bus, uint8_t channel, uint32_t speed)
{
	return WiringCPP::wiringCPPSPISetupMode(bus, channel, speed, 0);
}

int8_t WiringCPP::wiringCPPSPIDataRW(uint8_t bus, uint8_t channel, uint8_t *data, uint32_t len)
{
	try
	{
		WiringCPP::spidev& sd = WiringCPP::spidevmap.at((uint16_t)((bus << 8) | channel));
		
		struct spi_ioc_transfer xfer;
		
		memset(&xfer, 0, sizeof(xfer));
		
		xfer.tx_buf = (uint64_t) data;
		xfer.rx_buf = (uint64_t) data;
		xfer.len = len;
		xfer.speed_hz = sd.speed;
		xfer.delay_usecs = sd.delay;
		xfer.bits_per_word = sd.bpw;
		
		return ioctl(sd.fd, SPI_IOC_MESSAGE(1), &xfer);
	}
	catch(...)
	{
		return -1;
	}
}

int8_t WiringCPP::wiringCPPSPIClose(uint8_t bus, uint8_t channel)
{
	try
	{
		WiringCPP::spidev& sd = WiringCPP::spidevmap.at((uint16_t)((bus << 8) | channel));
		if(close(sd.fd)) return -2;
		if(WiringCPP::spidevmap.erase((uint16_t)((bus << 8) | channel)) != 1) return -3;
		return 0;
	}
	catch(...)
	{
		return -1;
	}
}

void WiringCPP::delay(uint32_t ms)
{
	usleep(ms * 1000);
}

void WiringCPP::delayu(uint32_t usec)
{
	usleep(usec);
}

int8_t WiringCPP::initTimer()
{
	try
	{
		struct timespec ts ;

		if(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0) return -2;
		epochMilli = (uint64_t)ts.tv_sec * (uint64_t)1000    + (uint64_t)(ts.tv_nsec / 1000000L) ;
		epochMicro = (uint64_t)ts.tv_sec * (uint64_t)1000000 + (uint64_t)(ts.tv_nsec /    1000L) ;
		return 0;
	}
	catch(...)
	{
		return -1;
	}
}

uint64_t WiringCPP::millis()
{
	try
	{
		struct timespec ts;
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0) return 0;
		
		uint64_t now = (uint64_t)ts.tv_sec * (uint64_t)1000 + (uint64_t)(ts.tv_nsec / 1000000L);
		return (now - epochMilli);
	}
	catch(...)
	{
		return 0;
	}
}

void WiringCPP::cleanup()
{
	//free all pins and chips
}