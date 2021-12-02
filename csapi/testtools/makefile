all: static
static:
	arm-linux-gcc -static -g -Wall  -o testtools testtools.c cmd.c util.c -I../include -I ../csgpio/include -I ../csfrontpanel/include -I ../cstvout/include -I ../csosd/include -I ../cshdmi/include -I ../csi2c/include -I ../csvid/include -I ../csaud/include -I ../cstuner_dvbs/include -I ../cstuner_dvbs/src -I ../cstuner/include -I ../cstuner/src  -I ../cssqc/include -I ../cssci/include -L../lib -lcsgpio -lcstvout -lcsfrontpanel -lcsosd -lcshdmi -lcsi2c -lcsvid -lcsaud -lcssqc -lcstuner_dvbs -lcstuner -lcssci -lpthread -lm -lcsevt

clean:
	rm -f testtools *.o 
