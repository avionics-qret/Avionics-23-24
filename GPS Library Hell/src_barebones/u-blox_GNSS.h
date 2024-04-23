#pragma once

#include <Arduino.h>
#include "u-blox_config_keys.h"
#include "u-blox_structs.h"
#include "u-blox_external_typedefs.h"
#include "u-blox_Class_and_ID.h"
#include "sfe_bus.h"

// Define a digital pin to aid debugging
// Leave set to -1 if not needed
const int debugPin = -1;

class DevUBLOXGNSS
{
public:
  DevUBLOXGNSS(void);   //constructor
  ~DevUBLOXGNSS(void);  //destructor



  /// Variables

  // Pointers to storage for the "automatic" messages
  // RAM is allocated for these if/when required.
  UBX_NAV_PVT_t *packetUBXNAVPVT = nullptr;             // Pointer to struct. RAM will be allocated for this if/when necessary

  /// Functions

  bool isConnected(uint16_t maxWait = kUBLOXGNSSDefaultMaxWait);
  
  // Send I2C/Serial/SPI commands to the module
  sfe_ublox_status_e sendCommand(ubxPacket *outgoingUBX, uint16_t maxWait = kUBLOXGNSSDefaultMaxWait, bool expectACKonly = false); // Given a packet and payload, send everything including CRC bytes, return true if we got a response

  // Val Setting
  bool setVal16(uint32_t key, uint16_t value, uint8_t layer = VAL_LAYER_RAM_BBR, uint16_t maxWait = kUBLOXGNSSDefaultMaxWait);           // Sets the 16-bit value at a given group/id/size location
  bool setValN(uint32_t key, uint8_t *value, uint8_t N, uint8_t layer = VAL_LAYER_RAM_BBR, uint16_t maxWait = kUBLOXGNSSDefaultMaxWait); // Sets the N-byte value at a given group/id/size location

  // Setting measurement / NAV rates
  bool setMeasurementRate(uint16_t rate, uint8_t layer = VAL_LAYER_RAM_BBR, uint16_t maxWait = kUBLOXGNSSDefaultMaxWait);       // Set the elapsed time between GNSS measurements in milliseconds, which defines the rate

  int32_t getLongitude(uint16_t maxWait = kUBLOXGNSSDefaultMaxWait);   // Returns the current longitude in degrees * 10-7. Auto selects between HighPrecision and Regular depending on ability of module.
  int32_t getLatitude(uint16_t maxWait = kUBLOXGNSSDefaultMaxWait);    // Returns the current latitude in degrees * 10^-7. Auto selects between HighPrecision and Regular depending on ability of module.

  bool getPVT(uint16_t maxWait = kUBLOXGNSSDefaultMaxWait);                                                                                                     // Query module for latest group of datums and load global vars: lat, long, alt, speed, SIV, accuracies, etc. If autoPVT is disabled, performs an explicit poll and waits, if enabled does not block. Returns true if new PVT is available.

  // These lock / unlock functions can be used if you have multiple tasks writing to the bus.
  // The idea is that in a RTOS you override this class and the functions in which you take and give a mutex.
  virtual bool createLock(void) { return true; }
  virtual bool lock(void) { return true; }
  virtual void unlock(void) {}
  virtual void deleteLock(void) {}

protected:

  /// Varaibles

  // Limit checking of new data to every X ms
  // If we are expecting an update every X Hz then we should check every quarter that amount of time
  // Otherwise we may block ourselves from seeing new data
  uint8_t i2cPollingWait = 100;    // Default to 100ms. Adjusted when user calls setNavigationFrequency() or setHNRNavigationRate() or setMeasurementRate()
  uint8_t i2cPollingWaitNAV = 100; // We need to record the desired polling rate for standard nav messages
  uint8_t i2cPollingWaitHNR = 100; // and for HNR too so we can set i2cPollingWait to the lower of the two

  // Init the packet structures and init them with pointers to the payloadAck, payloadCfg, payloadBuf and payloadAuto arrays  
  ubxPacket packetCfg = {0, 0, 0, 0, 0, payloadCfg, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED};

  // The packet buffers
  // These are pointed at from within the ubxPacket
  uint8_t *payloadCfg = nullptr;


 enum commTypes
  {
    COMM_TYPE_I2C = 0,
    COMM_TYPE_SERIAL,
    COMM_TYPE_SPI
  } _commType = COMM_TYPE_I2C; // Controls which port we look to for incoming bytes

  bool _printDebug = false;                      // Flag to print the serial commands we are sending to the Serial port for debug
  bool _printLimitedDebug = false;               // Flag to print limited debug messages. Useful for I2C debugging or high navigation rates

 

  /// Functions

  // The initPacket functions need to be private as they don't check if memory has already been allocated.
  // Functions like setAutoNAVPOSECEF will check that memory has not been allocated before calling initPacket.
  bool initPacketUBXNAVPVT();           // Allocate RAM for packetUBXNAVPVT and initialize it

  bool init(uint16_t maxWait, bool assumeSuccess);

};