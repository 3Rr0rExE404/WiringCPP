#ifndef WIRINGCPP_HPP
#define WIRINGCPP_HPP

#define	LOW 0
#define	HIGH 1

#define	INPUT 0
#define	OUTPUT 1

#include <inttypes.h>
#include <gpiod.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include <map>
#include <string>

#include <iostream>

namespace WiringCPP
{
	//// internals
	
	//controller id (CID) + pin id (PID) / 8 + 8 = 16 | gpio line ptr
	static std::map<uint16_t, struct gpiod_line*> pinmap;
	
	//
	class spidev 
	{
		public:
			std::string name;
			int fd;
			uint32_t speed;
			uint8_t bpw;
			uint16_t delay;
	};
	
	//spi master id + spi slave id | slave dev
	static std::map<uint16_t, spidev> spidevmap;
	
	//timer variables
	static uint64_t epochMilli = 0;
	static uint64_t epochMicro = 0;
	
	//// GPIO

	int8_t pinMode(uint8_t chip, uint8_t pin, uint8_t mode);
	int8_t digitalRead(uint8_t chip, uint8_t pin);
	int8_t digitalWrite(uint8_t chip, uint8_t pin, uint8_t state);
	int8_t pinClose(uint8_t chip, uint8_t pin);
	//close gpio chip

	//// SPI

	int8_t wiringCPPSPISetupMode(uint8_t bus, uint8_t channel, uint32_t speed, uint8_t mode);
	int8_t wiringCPPSPISetup(uint8_t bus, uint8_t channel, uint32_t speed); //default mode is 0
	int8_t wiringCPPSPIDataRW(uint8_t bus, uint8_t channel, uint8_t *data, uint32_t len);
	int8_t wiringCPPSPIClose(uint8_t bus, uint8_t channel);

	//// TIME

	void delay(uint32_t ms);
	void delayu(uint32_t usec);
	int8_t initTimer();
	uint64_t millis();
	
	//// CLEANUP
	
	void cleanup();
};

#endif