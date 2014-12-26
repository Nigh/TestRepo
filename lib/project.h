
#ifndef PROJECT_DEF
#define PROJECT_DEF

#define NECKMOVE_UPLIMIT (256*2.5)	// 2.5G
#define G_SLEEPLIMIT (256*0.8)	//0.8G

#define NECK_PITCH_POSITIVE_UPLIMIT (45)
#define NECK_PITCH_POSITIVE_DOWNLIMIT (5)
#define NECK_PITCH_NEGATIVE_UPLIMIT (-10)
#define NECK_PITCH_NEGATIVE_DOWNLIMIT (-70)

#define NECK_ROLL_POSITIVE_UPLIMIT (45)
#define NECK_ROLL_POSITIVE_DOWNLIMIT (5)
#define NECK_ROLL_NEGATIVE_UPLIMIT (-5)
#define NECK_ROLL_NEGATIVE_DOWNLIMIT (-45)

#define G_ACTIVE_TIMEOUT (600)	// x 0.1sec
#define STEP_ACTIVE_TIMEOUT (50)	// x 0.1sec

#define BLE_CONNECT_TIMEOUT_SET (10)	// x 1sec

#define HEALTHNECKMOVE (120)
#define TARGET_STEP (6000)

#define DATA_BEFORE_TIMESYNC (1)	// is data save without timesynced

#define FLASH_TEST_ADDR1 (0x16000)
#define FLASH_TEST_DATA1 (0xAA)

#define FLASH_TEST_ADDR2 (0x19000)
#define FLASH_TEST_DATA2 (0x55)

#define FLASH_TEST_ADDR3 (0x26000)
#define FLASH_TEST_DATA3 (0xA5)

#define MIN_DELTA_NECKMOVEMENT (0.9)
#define MIN_ZEROPOINT (5.5)

#define STR_VERSION "beta0.9.8E"	//v1.0.0

#endif
