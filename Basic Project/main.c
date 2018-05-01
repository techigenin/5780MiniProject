#include <atmel_start.h>
#include <string.h>

#define PORTA PORT->Group[0]
#define PORTB PORT->Group[1]
#define NUMHIGHS 8
#define NUMINS 8
// RESETTIME in ms
#define RESETTIME 250

static void setNextHigh();
static void sendByte(uint8_t);
static void checkAndSend(uint8_t, uint8_t);
static void resetIns();

int currentHigh;
char buttons[8][8];

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	// Setup a pin for control
	// Setup High Pins
	PORTA.DIRSET.reg = 1 << 2;
	PORTA.DIRSET.reg = 1 << 4;
	PORTA.DIRSET.reg = 1 << 5;

	PORTB.DIR.reg	|= 1 << 8 | 1 << 9 | 1 << 2 | 1 << 11 | 1 << 10;
	// Setup input/interrupt pins, pull down, EXTINT[6]
	PORTA.DIR.reg		 &= ~(1 << 17 | 1 << 19 | 1 << 16 | 1 << 18 | 1 << 7 | 1 <<  20 | 1 << 15 | 1 << 22);
	PORTA.OUT.reg		 &= ~(1 << 17 | 1 << 19 | 1 << 16 | 1 << 18 | 1 << 7 | 1 <<  20 | 1 << 15 | 1 << 22);
	PORTA.PINCFG[17].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[19].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[16].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[18].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[07].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[20].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[15].reg |= 1 << 2 | 1 << 1 | 1 << 0;
	PORTA.PINCFG[22].reg |= 1 << 2 | 1 << 1 | 1 << 0;

	// Setup timer interrupt
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_ID_TCC2_TC3 | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
	REG_PM_APBCMASK |= PM_APBCMASK_TC3; // Enable TCC0 in PM
	TC3->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_ENABLE; // Setup Timer
	TC3->COUNT16.INTENSET.reg  |= TC_INTENSET_MC0;
	TC3->COUNT16.CC[0].reg	= (int)(RESETTIME/1.024);// Set Top value (100ms / (1µs * 1024) = 97)
	
	NVIC_EnableIRQ(TC3_IRQn);
	NVIC_SetPriority(TC3_IRQn, 3);

	// Setup Interrupt for Pin PA22
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
	REG_PM_APBAMASK |= PM_APBAMASK_EIC;
	EIC->INTENSET.reg	= EIC_INTENSET_EXTINT0 |
							EIC_INTENSET_EXTINT1 | 
							EIC_INTENSET_EXTINT2 | 
							EIC_INTENSET_EXTINT3 | 
							EIC_INTENSET_EXTINT4 | 
							EIC_INTENSET_EXTINT6 | 
							EIC_INTENSET_EXTINT7 | 
							EIC_INTENSET_EXTINT15;
	EIC->CONFIG[0].reg 	|=  EIC_CONFIG_FILTEN0 | EIC_CONFIG_SENSE0_RISE |
							EIC_CONFIG_FILTEN1 | EIC_CONFIG_SENSE1_RISE |
							EIC_CONFIG_FILTEN2 | EIC_CONFIG_SENSE2_RISE |
							EIC_CONFIG_FILTEN3 | EIC_CONFIG_SENSE3_RISE |
							EIC_CONFIG_FILTEN4 | EIC_CONFIG_SENSE4_RISE |
							EIC_CONFIG_FILTEN6 | EIC_CONFIG_SENSE6_RISE |
							EIC_CONFIG_FILTEN7 | EIC_CONFIG_SENSE7_RISE;
	EIC->CONFIG[1].reg	|=	EIC_CONFIG_FILTEN7 | EIC_CONFIG_SENSE7_RISE; // EXTINT[15]
	EIC->CTRL.reg		|= EIC_CTRL_ENABLE; // Enable

	// Setup USART 
	// If I decide to setup callbacks, they go here
	usart_async_enable(&USART_0);
	currentHigh = 0;

	NVIC_EnableIRQ(EIC_IRQn);
	NVIC_SetPriority(EIC_IRQn, 3);
	NVIC_SetPriority(SERCOM2_IRQn, 2);

	resetIns();

	// Loops through the high pins
	while (1) {
		if (currentHigh >= 8)
			currentHigh = 0;
		setNextHigh();

		delay_us(125); 
		currentHigh++;
	}
}

/* 
 * Each input can interrupt the main loop
 * When it does, it calculates which pin it really is, and sends it up UART
 */
void EIC_Handler()
{	
	if (EIC->INTFLAG.reg & 1 << 1)
	{ 
		checkAndSend(currentHigh, 0);
		EIC->INTFLAG.reg |= 1 << 1;
	}
	 if (EIC->INTFLAG.reg & 1 << 3)
	{
		checkAndSend(currentHigh, 1);
		EIC->INTFLAG.reg |= 1 << 3;
	}
	 if (EIC->INTFLAG.reg & 1 << 0)
	{
		checkAndSend(currentHigh, 2);
		EIC->INTFLAG.reg |= 1 << 0;
	}
	 if (EIC->INTFLAG.reg & 1 << 2)
	{
		checkAndSend(currentHigh, 3);
		EIC->INTFLAG.reg |= 1 << 2;
	}
	 if (EIC->INTFLAG.reg & 1 << 7)
	{
		checkAndSend(currentHigh, 4);
		EIC->INTFLAG.reg |= 1 << 7;
	}
	 if (EIC->INTFLAG.reg & 1 << 4)
	{
		checkAndSend(currentHigh, 5);
		EIC->INTFLAG.reg |= 1 << 4;
	}
	 if (EIC->INTFLAG.reg & 1 << 15)
	{
		checkAndSend(currentHigh, 6);
		EIC->INTFLAG.reg |= 1 << 15;
	}
	 if (EIC->INTFLAG.reg & 1 << 6)
	{
		checkAndSend(currentHigh, 7);
		EIC->INTFLAG.reg |= 1 << 6;
	}
}

void TC3_Handler()
{
	if (TC3->COUNT16.INTFLAG.reg & TC_INTFLAG_MC0)
	{
		resetIns();
		TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0;
	}
}

void resetIns()
{
	int i, j;

	for (i = 0; i < NUMHIGHS; i++)
		for(j = 0; j < NUMINS; j++)
			buttons[i][j] = 0;
}


void checkAndSend(uint8_t base, uint8_t val)
{
	if (buttons[base][val] == 0)
	{	
		TC3->COUNT16.COUNT.reg = 0; // Restart Counter
		uint8_t whichPin = (base  << 3) + val;
		sendByte(whichPin);
		buttons[base][val] = 1;
	}
}

void setNextHigh()
{
	switch(currentHigh)
	{
		case 0:
			PORTB.OUTCLR.reg = 1 << 10;
			PORTA.OUTSET.reg = 1 << 2;
			break;
		case 1:
			PORTA.OUTCLR.reg = 1 << 2;
			PORTB.OUTSET.reg = 1 << 8;
			break;
		case 2:
			PORTB.OUTCLR.reg = 1 << 8;
			PORTB.OUTSET.reg = 1 << 9;
			break;
		case 3:
			PORTB.OUTCLR.reg = 1 << 9;
			PORTA.OUTSET.reg = 1 << 4;
			break;
		case 4:
			PORTA.OUTCLR.reg = 1 <<  4;
			PORTA.OUTSET.reg = 1 << 5;
			break;
		case 5:
			PORTA.OUTCLR.reg = 1 << 5;
			PORTB.OUTSET.reg = 1 << 2;
			break;
		case 6:
			PORTB.OUTCLR.reg = 1 << 2;
			PORTB.OUTSET.reg = 1 << 11;
			break;
		case 7:
			PORTB.OUTCLR.reg = 1 << 11;
			PORTB.OUTSET.reg = 1 << 10;
			break;
		default:
			break;
	}
}

static void sendByte(uint8_t data)
{
	/* 
	 * Wait until the DRE register is set
	 * Then write the byte to the data register
	 * Then wait until the TXC register is set.
	 */
	while((SERCOM2->USART.INTFLAG.reg & 1) == 0){}
	SERCOM2->USART.DATA.reg = (data & 0xff);
	while((SERCOM2->USART.INTFLAG.reg & (1 << 1))==0){}
	
}
