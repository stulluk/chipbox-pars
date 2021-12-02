/****************************************************************
*
* FILENAME
*	caskey.h
*
* PURPOSE 
*	Cas Driver Key Data Header
*	
*
* AUTHOR
*	Jacob
*
* HISTORY
*  Status                            Date              Author
*  Create                        14 July 2005          Jacob
*
****************************************************************/
#ifndef  _CAS_KEY_H_INCLUDED
#define  _CAS_KEY_H_INCLUDED
/****************************************************************
 *                          define                              *
 ****************************************************************/

/* Key Data Test */
#define  KEY_DATA_TEST

#define  KEY_DB_LENGTH_FIELD_SIZE  2
#define  KEY_FLASH_ID_LENGTH       8

#define  MIN_FLASH_LENGTH          (KEY_FLASH_ID_LENGTH + KEY_DB_LENGTH_FIELD_SIZE + 1) // length field + Number of Cas + KeydataID
#define  KEY_FLASH_SIZE            0x10000

#define  NULL_KEY_NUMBER           0xFF

/****************************************************************
 *	Type Define                                                 *
 ****************************************************************/
typedef struct KeyData_s
{
	struct KeyData_s       *NextKeyData;
	unsigned char           KeyNumber;
	unsigned char           KeyLength;
	unsigned char           *KeyData;
} KeyData_t;

typedef struct ProviderData_s
{
	struct ProviderData_s  *NextProviderData;
	unsigned char           ProviderId[CAS_PROVIDER_ID_LENGTH];
	unsigned char           ProviderName[CAS_PROVIDER_NAME_LENGTH];
	unsigned char           TotalNumberOfKey;
	KeyData_t              *Key;
} ProviderData_t;

typedef struct CasData_s
{
	struct CasData_s       *NextCasData;
	unsigned char           CasId[CAS_ID_LENGTH];
	unsigned char           CasName[CAS_NAME_LENGTH];
	unsigned char           TotalNumberOfProvider;
	unsigned char           ProviderIdLen;
	ProviderData_t         *Provider;
} CasData_t;

typedef struct CasDbInfo_s
{
	struct CasData_s       *CasData_P;
	unsigned char           NumberOfCas;
} CasDbInfo_t;

typedef struct CasSystemInfo_s
{
	U16        CasId;
	U8         CasName[20];
} CasSystemInfo_t;


/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

#endif // #ifndef  _CAS_KEY_H_INCLUDED
