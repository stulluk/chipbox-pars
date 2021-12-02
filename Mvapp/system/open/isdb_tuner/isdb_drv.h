#ifndef ISDB_ALPUS_DRV_H_
#define ISDB_ALPUS_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TC90507_BIT_0					0x01
#define TC90507_BIT_1					0x02
#define TC90507_BIT_2					0x04
#define TC90507_BIT_3					0x08
#define TC90507_BIT_4					0x10
#define TC90507_BIT_5					0x20
#define TC90507_BIT_6					0x40
#define TC90507_BIT_7					0x80

#define TC90507_CH_SELECTION_MODE		0
#define TC90507_CH_SEARCH_MODE			1

// TC90507_ERROR_Status_t
#define TC90507_RS_ERROR				0
#define TC90507_RS_NONERROR				1

#define TC90507_CONFIG_DIGITAL			0
#define TC90507_CONFIG_ANALOG			1

// TC90507_TSOUT_Enable_t;
#define TC90507_ENABLE					0
#define TC90507_DISABLE					1


// TC90507_TSOUT_Enable_t;
#define TC90507_ENABLE					0
#define TC90507_DISABLE					1

#define ALP514_DATA_SIZE				6
/// ALP514_DATA_FORMAT_t
#define ALP514_INDEX_ADDRESS			0
#define ALP514_INDEX_DIVIDER1			1	// programmable LO frequency
#define ALP514_INDEX_DIVIDER2			2
#define ALP514_INDEX_CONTROL1			3	// AGC take-over(start) point, reference divider ratio
#define ALP514_INDEX_CONTROL2			4	// charge pump current_10, Test bits_32, Band Switch
#define ALP514_INDEX_CONTROL3			5	// IFTRAP switch, charge pump current_2, Power-save mode control

// TC90507_BERMONITOR_Data_t
#define TC90507_BER_AFTER_RS			0
#define TC90507_BER_AFTER_VITERBI		1

#define TC90507_TS_OUTPUT_LAYER_ALL		0
#define TC90507_TS_OUTPUT_LAYER_A		1
#define TC90507_TS_OUTPUT_LAYER_B		2
#define TC90507_TS_OUTPUT_LAYER_C		3

typedef struct
{
	U8 ucStatus_LayerA;
	U8 ucStatus_LayerB;
	U8 ucStatus_LayerC;
} TC90507_ucRSERROR_t;

typedef struct
{
	double dBER_LayerA;
	double dBER_LayerB;
	double dBER_LayerC;
} TC90507_LAYER_dBER_t;

typedef struct
{
	char *pcLayerA;
	char *pcLayerB;
	char *pcLayerC;
} TC90507_pcLAYER_t;

typedef struct
{
	TC90507_pcLAYER_t tMod;		// TMCC layer X carrier modulation system
	TC90507_pcLAYER_t tCR;			// TMCC layer X convolution coding rate
	TC90507_pcLAYER_t tTI;			// TMCC layer X time interleave system
	TC90507_pcLAYER_t tNumSeg;		// Number of segments used by layer X
} TC90507_pcLAYER_INFOR_t;


#ifdef __cplusplus
}
#endif

#endif