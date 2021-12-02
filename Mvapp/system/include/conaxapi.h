#ifndef _CONAX_API_H_
#define _CONAX_API_H_

enum{
		TS_LEVEL,
		PES_LEVEL
};

void Conax_init(void);
void Conax_open(unsigned short int Pid);
void Conax_StartPMT(unsigned short int Pid);
void Conax_PMTDescriptor(unsigned char const*buf);
#endif
