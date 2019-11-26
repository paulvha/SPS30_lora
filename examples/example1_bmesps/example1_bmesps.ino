/************************************************************************************
 *  Copyright (c) October 2019, version 1.1     Paul van Haastrecht
 *
 *  Version 1.2 / November 2019
 *  - changed data order to make BME280 data optional (in case not found)
 *
 *  =========================  Highlevel description ================================
 *
 *  This basic reading example sketch to connect an SPS30 and BME280 for providing data, and
 *  sent to TTN.
 *
 *  This sketch was developed and tested on Feather Lora 32U4 where it ONLY WORKS works I2C COMMUNICATION.
 *  It has been optimised to just fit the program space (32KB)
 *
 *  =========================  Hardware connections =================================
 *
 *  FOR DETAILED INFORMATION SEE THE FEATHER_LORA.ODT IN THE EXTRAS DIRECTORY
 *
 *  Arduino feather 32U4
 *
 *   BME280        Arduino feather     (ORIGINAL ARDUINO ASSUMED !!  see note 1)
 *    VIN --------- 5V                 (see Note 2 and Note 3)
 *    SDI --------- SDA PO 25
 *    SCK --------- SCL PO 26
 *    GND --------- GND
 *
 *    3V3  if available NOT CONNECTED
 *    SDO  if available See Note 1
 *    CS   if available NOT CONNECTED
 *
 *
 *   SPS30 pin     Arduino feather     (see Note 3)
 *    1 VCC -------- VUSB
 *    2 SDA -------- SDA
 *    3 SCL -------- SCL
 *    4 Select ----- GND (select I2c)
 *    5 GND -------- GND
 *
 *  WARNING !!!!!
 *  The SPS30 MUST have pull-up resistor of 10K each on the SCL and SDA line. See Note 3.
 *
 *  Note 1:
 *  ORIGINAL ARDUINO ASSUMED, but can be another version !!
 *  By default on the BME280, the I2C address is 0x77. If you add a jumper from SDO to GND, the address will change to 0x76.
 *  Make sure to select the RIGHT BME280 address around line 153
 *
 *  Note 2:
 *  An original Arduino BME280 has on-board 3V3 regulator. If you have another one that only supports 3V3, you have
 *  to have select 3V3 for the voltage, else you can select USB power as well. See hardware connection in the lora feather.odt
 *
 *  Note 3:
 *  In the extra directory is documentation for different Hardware connection and use of
 *  BME280 as 5V or 3V3 and the need for pull-up resistors.
 *
 *  ================================= PARAMETERS =====================================
 *
 *  From line 93 there are configuration parameters for the program
 *
 *  ================================== SOFTWARE ======================================
 *
 *  The following libraries are necessary to be installed:
 *  LMIC : https://github.com/matthijskooijman/arduino-lmic
 *
 *  Using standard libraries :
 *    BME : Adafruit_BME280 and Adafruit_Sensor
 *    SPS30 : sps (https://github.com/paulvha/sps30)
 *  else :
 *    BME280 & sps30: lora_bme_sps (https://github.com/paulvha/SPS30_lora)
 *
 *  ================================ Disclaimer ======================================
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *                            *************************
 *
 *  The lMIC code is based on ttn_ABP sketch from Thomas Telkamp and Matthijs Kooijman
 *
 *  Permission is hereby granted, free of charge, to anyone
 *  obtaining a copy of this document and accompanying files,
 *  to do whatever they want with them without any restriction,
 *  including, but not limited to, copying, modification and redistribution.
 *  NO WARRANTY OF ANY KIND IS PROVIDED.
 *  ===================================================================================
 *
 *  NO support, delivered as is, have fun, good luck !!
 */

#include <lmic.h>
#include <hal/hal.h>          // needed for lmic_pins

////////////////////////////////////////////////////////////////////////////////////////
/*  Due to memory constraints a special cut-down version of the BME280 and SPS30
 *  library was created call lora_bme_sps. With this library the program memory is
 *  used for 91% (or 95% with SKETCH_DEBUG).
 *
 *  However with I2C you can also use the standard libraries resulting in 95% program
 *  memory usage (99% with SKETCH_DEBUG.)
 *
 *  Advice is to use the standard libraries as updates/bug fixes may come in the future that
 *  are important for this implementation. The special Lora_bme_sps library is not planned
 *  to be maintained on a regular base. BUT if you need to add more code and use SKETCH_DEBUG
 *  you might want to use lora_bme_sps.
 *
 *  By default the comments are included (//) before the define statement. By removing them
 *  you switch from the standard libraries to the special lora one (which you must have installed of course)
 *///////////////////////////////////////////////////////////////////////////////////
//#define USE_LORA_SPECIAL 1

/////////////////////////////////////////////////////////////
/*define communication channel to use for SPS30
 valid option for Feather LORA is:
 *   I2_COMMS          use I2C communication  */
/////////////////////////////////////////////////////////////
#define SP30_COMMS I2C_COMMS

/////////////////////////////////////////////////////////////
/* define SPS30 driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
 //////////////////////////////////////////////////////////////
#define SPS30_DEBUG 0

// Set to 1 for sketch debug / result information
#define SKETCH_DEBUG 0

// Schedule TX every this many seconds (Is not precize and might become longer due to duty cycle).
#define TX_INTERVAL  170
#define MEAS_BME TX_INTERVAL - 5   // Meassure Temp/RH/Pressure atleast 5 sec before sending

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
///////////////////////// LMIC  & BME280 parameter setting ///////////////////

// LoRaWAN NwkSKey, network session key from TTN (use the same format as this example)
static const PROGMEM u1_t NWKSKEY[16] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
// LoRaWAN AppSKey, application session key From TTN (use the same format as this example)
static const u1_t PROGMEM APPSKEY[16] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
// LoRaWAN end-device address (DevAddr) from TTN (use the same format as this example)
static const u4_t DEVADDR = 0x03FF0001;

// Pin mapping (
// WARNING !!!! make sure to connect IO1 (next to TX) to pin 6 (physical pin 2, second pin from SCL pin) !!!!
const lmic_pinmap lmic_pins = {
  .nss = 8,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 4,
  .dio = {7, 6, LMIC_UNUSED_PIN},
};

// BME280 address
#define I2C_ADDRESS 0x76

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

// create constructors
#if defined USE_LORA_SPECIAL

#include <lora_sps30.h>
#include <lora_BME280.h>

lora_SPS30 sps30;
lora_BME280 bme;

#else

#include <sps30.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

SPS30 sps30;
Adafruit_BME280 bme;

#endif

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;     // needed in do_send

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
////////////////////////////// PROGRAM VARIABLES ////////////////////////////
int  NumLoop = 0;           // loop counter to include BME280 values

bool status_BME = false;    // BME280 detected ?
float BME_Temp = 0.0;       // store BME temperature
float BME_Pres = 0.0;       // store BME pressure
float BME_Humi = 0.0;       // store BME humidity

unsigned int SPS_id = 0;    // SPS30 ID to include in message to TTN

unsigned int Counter_Pm = 0;// number of samples in total
float fPm1_Total = 0.0, fPm10_Total = 0.0, fPm25_Total = 0.0;
int16_t temp_int,rh_int ,p_int;
int16_t pm10_Avg_int, pm25_Avg_int, pm1_Avg_int;

// multi-purpose buffer
uint8_t buffer[32];

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/**
 * @brief: call back from LMIC library
 */
void onEvent (ev_t ev) {

  //Serial.println("onEvent ");

  Serial.print(os_getTime());
  Serial.print(":");

  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println("EV_SCAN_TIMEOUT");
      break;
    case EV_BEACON_FOUND:
      Serial.println("EV_BEACON_FOUND");
      break;
    case EV_BEACON_MISSED:
      Serial.println("EV_BEACON_MISSED");
      break;
    case EV_BEACON_TRACKED:
      Serial.println("EV_BEACON_TRACKED");
      break;
    case EV_JOINING:
      Serial.println("EV_JOINING");
      break;
    case EV_JOINED:
      Serial.println("EV_JOINED");
      break;
    case EV_RFU1:
      Serial.println("EV_RFU1");
      break;
    case EV_JOIN_FAILED:
      Serial.println("EV_JOIN_FAILED");
      break;
    case EV_REJOIN_FAILED:
      Serial.println("EV_REJOIN_FAILED");
      break;
    case EV_TXCOMPLETE:
      Serial.println("EV_TXCOMPLETE (includes waiting for RX windows)");
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println("Received ack");
      if (LMIC.dataLen) {
        Serial.println("Received ");
        Serial.println(LMIC.dataLen);
        Serial.println(" bytes of payload");
      }
      // Schedule next transmission, this does not mean it is handled right away
      // So calculate now and sent this later with do_send()!!!
      calculate_data();
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println("EV_LOST_TSYNC");
      break;
    case EV_RESET:
      Serial.println("EV_RESET");
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println("EV_RXCOMPLETE");
      break;
    case EV_LINK_DEAD:
      Serial.println("EV_LINK_DEAD");
      break;
    case EV_LINK_ALIVE:
      Serial.println("EV_LINK_ALIVE");
      break;
    default:
      Serial.println("Unknown event");
      break;
  }
}

/**
 * @brief : calculate the results to be sent later
 */
void calculate_data()
{
  temp_int = BME_Temp * 100;        // 24.43 becomes 2443
  rh_int = BME_Humi * 100 ;         // 43.50 becomes 4350
  p_int = BME_Pres;                 // 1006.73 becomes 1006

  pm10_Avg_int = (fPm10_Total / Counter_Pm) * 100;  // 17.24 becomes 1724
  pm25_Avg_int = (fPm25_Total / Counter_Pm) * 100;  // 12.48 becomes 1248
  pm1_Avg_int =  (fPm1_Total  / Counter_Pm) * 100;  // 9.24 becomes 924

  // Zero Counters
  NumLoop = 0;
  fPm10_Total = fPm25_Total = fPm1_Total  = 0;
  Counter_Pm = 0;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/**
 * @brief : read data from SPS30 and calculate totals
 */

void ProcessData()
{
  struct sps_values val;        // store SPS30 values

  // read SPS30 values
  if(! read_SPS30(&val)) return;

  fPm1_Total += val.MassPM1;
  fPm25_Total += val.MassPM2;
  fPm10_Total += val.MassPM10;
  Counter_Pm++;

  // optimized to fit in program memory
  if (SKETCH_DEBUG) {
      Serial.print("SPS_id; ");
      Serial.println(SPS_id, HEX);

      disp_info("\nCounter_Pm:", &Counter_Pm, false);
      disp_info("PM1 Total  :", &fPm1_Total, true);
      disp_info("PM2.5 Total:",&fPm25_Total, true);
      disp_info("PM10 Total :", &fPm10_Total, true);

      disp_info("\nPm1_Sensor  :", &val.MassPM1, true);
      disp_info("Pm2.5_Sensor:",&val.MassPM2, true);
      disp_info("Pm10_Sensor :", &val.MassPM10, true);

      disp_info("\nTemp_Sensor:", &BME_Temp, true);
      disp_info("Pres_Sensor:", &BME_Pres, true);
      disp_info("Humi_Sensor:", &BME_Humi, true);
      Serial.println();
  }
}

/**
 * @brief : prepare sent buffer and send it
 */
void do_send(osjob_t* j) {
  //Serial.println(F("ENTER do_send"));

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
    return;
  }


  // Prepare upstream data transmission at the next possible time.
  // order changed in version 1.2 to make BME data optional
  buffer[0]  = SPS_id >> 8;
  buffer[1]  = SPS_id;
  buffer[2]  = pm10_Avg_int >> 8;
  buffer[3]  = pm10_Avg_int;
  buffer[4]  = pm25_Avg_int >> 8;
  buffer[5]  = pm25_Avg_int;
  buffer[6]  = pm1_Avg_int >> 8;
  buffer[7]  = pm1_Avg_int;
  buffer[8]  = temp_int >> 8;         //moved to the back as it might be optional
  buffer[9]  = temp_int;
  buffer[10]  = rh_int >> 8;
  buffer[11]  = rh_int; ;
  buffer[12]  = p_int >> 8;
  buffer[13]  = p_int;

  LMIC_setTxData2(1, buffer, 14, 0);

  if (SKETCH_DEBUG) {
    Serial.print("\t\tPacket queued\nSds_ID: ");
    Serial.println(SPS_id, HEX);
    disp_info("Temp:\t", &temp_int, false);
    disp_info("RH:\t", &rh_int, false);
    disp_info("Pressure:", &p_int, false);
    disp_info("PM10_Avg:",&pm10_Avg_int, false);
    disp_info("PM2.5_Avg:",&pm25_Avg_int, false);
    disp_info("PM1_Avg:",&pm1_Avg_int, false);
  }

  // Next TX is scheduled after TX_COMPLETE event.
}

/**
 *  @brief : display information to reduce the number of Serial.print() for memory reasons
 *  @param m : message to display
 *  @param *val : pointer to value to display
 *  @type : if true display val as float, else display as uint16_t
 *
 */
void disp_info(char * m, void *val, bool type)
{
   Serial.print(m);
   Serial.print("\t");
   if(type) {
    float ft = *(float *) val;
    Serial.println(ft);
   }
   else {
    uint16_t t = *(uint16_t *) val;
    Serial.print(t);
    Serial.print("\t0x");           // Hex output to compare against TTN
    Serial.println(t, HEX);         // console info
  }
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void setup() {

  Serial.begin(115200);

  /* ADDED WHILE(!Serial) TO WAIT FOR SERIAL CONSOLE TO BE AVAILABLE AND
   * DISPLAY THE COMPLETE INFORMATION FROM START. IT ALSO HELPED DURING UPLOAD
   *
   * IF YOU ARE APPLY POWER TO THE VUSB WITHOUT CONNECTING A
   * SERIAL MONITOR, COMMENT THIS LINE OUT OTHERWISE THE SKETCH
   * DOES NOT GO BEYOND THIS POINT.
   */

  while(!Serial);

///////////////////// SPS30 init /////////////////////////////

  Serial.print("connecting to SPS30: ");

  // set driver debug level
  sps30.EnableDebugging(SPS30_DEBUG);

  // Begin communication channel;
  if (! sps30.begin(SP30_COMMS))
    Errorloop("could not initialize communication channel.");

  // reset SPS30 connection
  if (! sps30.reset())  Errorloop("could not reset.");

  // read device serial number
  GetDeviceInfo();

  // start measurement
  if ( sps30.start() ) Serial.println("SPS30 started");
  else Errorloop("Could NOT start SPS30");

///////////////////// LMIC init /////////////////////////////

#ifdef VCC_ENABLE
  // For Pinoccio Scout boards
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
#endif

  // LMIC init
  os_init();

  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

#if defined(CFG_eu868)
  // this VERIABLE is defined in config.h
  // *************************************************************
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  // NA-US channels 0-71 are configured automatically

  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.
#elif defined(CFG_us915)
  // NA-US channels 0-71 are configured automatically
  // but only one group of 8 should (a subband) should be active
  // TTN recommends the second sub band, 1 in a zero based count.
  // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
  LMIC_selectSubBand(1);
#endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF12, 14);

  // Start job
  do_send(&sendjob);

///////////////////// BME280 init /////////////////////////////

  Serial.print("\nBME280 test: ");
  status_BME = bme.begin(I2C_ADDRESS);
  if (!status_BME) Serial.println("Did not find the sensor, check wiring!");
  else Serial.println("Detected !");
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void loop() {

  Serial.print(NumLoop);        // needed ?
  Serial.print(" ");            // needed ?

  if (NumLoop >= MEAS_BME){     // After X counts start read BME Sensor

    // if BME was detected
    if (status_BME) {
      BME_Temp = bme.readTemperature();
      BME_Pres = bme.readPressure() / 100.0F;
      BME_Humi = bme.readHumidity();
    }
  }

  delay(1000);       // This is a one sec delay.  1000 = 1000 msec

  ProcessData();     // read from SPS30 the calculate totals

  os_runloop_once(); // trigger LMIC library

  NumLoop++;
}

/**
 * @brief : read serial number
 */
void GetDeviceInfo()
{
  //try to read serial number
  if (sps30.GetSerialNumber((char *) buffer, 16) == ERR_OK) {

      // use the last 4 digits
      for (uint8_t i = 12; i < 16; i++)
      {
       uint8_t tmp =  atou(i++) << 4;
       tmp = tmp | atou(i);
       SPS_id = (SPS_id << 8) | tmp;
      }
  }
  else
    Errorloop("could not get serial number");
}

/**
 * @brief Ascii character to hex nibble
 * @param i : offset for ASCII character in the generic buffer
 */
uint8_t atou(uint8_t i)
{
  if (buffer[i] <= '9') return(buffer[i] - '0');
  else return(buffer[i]-'A' + 0x0a);
}

/**
 * @brief : read SPS30 values
 * @param val : pointer to structure to store values
 */
bool read_SPS30(struct sps_values *val)
{
  uint8_t ret, error_cnt = 0;

  // loop to get data
  do {

    ret = sps30.GetValues(val);

    // data might not have been ready
    if (ret == ERR_DATALENGTH){
        if (error_cnt++ > 3) {
          Errorloop("Error during reading values");
          return(false);
        }
        delay(500);
    }

    // if other error
    else if(ret != ERR_OK) {
      Errorloop("Error during reading values");
      return(false);
    }

  } while (ret != ERR_OK);

  return(true);
}

/**
 *  @brief : continued loop after fatal error
 *  @param mess : message to display
 */
void Errorloop(char *mess)
{
  Serial.println(mess);
  Serial.println("Program on hold");
  for(;;) delay(100000);
}
