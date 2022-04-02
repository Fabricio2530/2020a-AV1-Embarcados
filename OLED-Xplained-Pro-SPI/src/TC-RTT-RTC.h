
#include "TC-RTT-RTC.c"

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);

