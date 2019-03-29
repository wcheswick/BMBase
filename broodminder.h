// le_advertising_info response to lescan, found in info->data, length info->length
//
//

// broodminder stuff is at 14: 
// 02 01 06 02 0a 03 18 ff 8d 02 2a 38 02 00 5a f2 00 30 5f 00 00 00 00 00 20 9f 0a 42 00 00 00 ae
// 1) Check for "Manufacturer Specific Data"
//         Bytes 6,7 = 0x18, 0xff
// 2) Check for IF, LLC as the manufacturer
//         Bytes 8,9 = 0x8d, 0x02
// 3) Bytes 10-29 are the data from the BroodMinder as outlined below.
//         deviceModelIFllc_1 = 2b (43d = scale)
//         DeviceVersionMinor_1 = 15 (21d) 
//         DeviceVersionMajor_1  = 02 (FW 2.21)
//         Battery_1V2 = 2%
//         Elapsed_2V2 = 21 (33d) 
//         Temperature_2V2 = 62d0
//         WeightL_2V2 = 7FFF
//         WeightR_2V2 = 8005
//         Humidity_1V2 = 37
//         UUID_3V2 = b5:30:07

// bytes in evt_le_meta_event

#define IS_BROODMINDER(info)	\
	(((info)[6] == 0x18) && \
	((info)[7] == 0xff) && \
	((info)[8] == 0x8d) && \
	((info)[9] == 0x02))

// The broodminder data is not aligned nicely, and the "packed"
// attribute seems to be non-functional, so all these defines are
// for byte addresses, and the USHORT macro used to assemble endian-correct
// values.

typedef struct bm_adv_resp {
	uint8_t	bm_model;
	uint8_t	bm_devminor;
	uint8_t	bm_devmajor;
	uint8_t bm_unused13;
	union {
		struct v1 {
			uint8_t	bm_battery;
			uint8_t	bm_samples;
			uint8_t	bm_samples_2;
			uint8_t	bm_temp;
			uint8_t	bm_temp_2;
			uint8_t	bm_temp14d_ave;
			uint8_t	bm_temp14d_min;
			uint8_t	bm_temp14d_min_2;
			uint8_t	bm_temp14d_max;
			uint8_t	bm_temp14d_max_2;
			uint8_t	bm_humidity;
			uint8_t	bm_hum14d_ave;
			uint8_t	bm_hum14d_min;
			uint8_t	bm_hum14d_min_2;
			uint8_t	bm_hum14d_max;
			uint8_t	bm_hum14d_max_2;
		} v1;
		struct v2 {
			uint8_t	bm_battery;
			uint8_t	bm_samples;	// "elapsed"
			uint8_t	bm_samples_2;
			uint8_t	bm_temp;
			uint8_t	bm_temp_2;
			uint8_t bm_unused19;
			uint8_t	bm_weight_left;
			uint8_t	bm_weight_left_2;
			uint8_t	bm_weight_right;
			uint8_t	bm_weight_right_2;
			uint8_t	bm_humidity;
			u_char	bm_UUID[3];
		} v2;
	};
} bm_adv_resp;

#define USHORT(a)	((uint16_t)(((a))) + (*(&(a)+1)<<8))

// Compute degrees F:

#define F(s)	(((double)(USHORT(s)) / (double)0x10000*165.0 - 40.0)*(9.0/5.0) + 32.0)

// values for bm_model:

#define BM_SENSOR	42
#define BM_SCALE	43

extern	int bm_f(u_short tr);
extern	double bm_w(uint8_t *b);
