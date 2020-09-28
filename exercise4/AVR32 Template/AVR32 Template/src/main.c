#include <asf.h>
#include <board.h>
#include <gpio.h>
#include <sysclk.h>
#include "busy_delay.h"

#define CONFIG_USART_IF (AVR32_USART2)

// defines for BRTT interface
#define TEST_A      AVR32_PIN_PA31
#define RESPONSE_A  AVR32_PIN_PA30
#define TEST_B      AVR32_PIN_PA29
#define RESPONSE_B  AVR32_PIN_PA28
#define TEST_C      AVR32_PIN_PA27
#define RESPONSE_C  AVR32_PIN_PB00

volatile int interrupt_flag = 0;


__attribute__((__interrupt__)) static void interrupt_J3(void);

void init(){
    sysclk_init();
    board_init();
    busy_delay_init(BOARD_OSC0_HZ);
    
    cpu_irq_disable();
    INTC_init_interrupts();
    INTC_register_interrupt(&interrupt_J3, AVR32_GPIO_IRQ_3, AVR32_INTC_INT1);
    cpu_irq_enable();
    
    stdio_usb_init(&CONFIG_USART_IF);

    #if defined(__GNUC__) && defined(__AVR32__)
        setbuf(stdout, NULL);
        setbuf(stdin,  NULL);
    #endif
}

__attribute__((__interrupt__)) static void interrupt_J3(void){ 
	if ( gpio_get_pin_interrupt_flag(TEST_A) ) {
		gpio_clear_pin_interrupt_flag(TEST_A);
		//gpio_clr_gpio_pin(RESPONSE_A);
		//busy_delay_us(5);
		//gpio_set_gpio_pin(RESPONSE_A);
		interrupt_flag |= 0x1;
	}
	if ( gpio_get_pin_interrupt_flag(TEST_B) ) {
		//busy_delay_us(100);
		gpio_clear_pin_interrupt_flag(TEST_B);
		//gpio_clr_gpio_pin(RESPONSE_B);
		//busy_delay_us(5);
		//gpio_set_gpio_pin(RESPONSE_B);
		interrupt_flag |= 0x2;
	}
	if ( gpio_get_pin_interrupt_flag(TEST_C) ) {
		gpio_clear_pin_interrupt_flag(TEST_C);
		//gpio_clr_gpio_pin(RESPONSE_C);
		//busy_delay_us(5);
		//gpio_set_gpio_pin(RESPONSE_C);
		interrupt_flag |= 0x4;
	}
}


int main (void){
    init();
	
    // Init task A pins
    gpio_configure_pin(TEST_A, (GPIO_DIR_INPUT | GPIO_INIT_HIGH));
	gpio_configure_pin(TEST_B, (GPIO_DIR_INPUT | GPIO_INIT_HIGH));
	gpio_configure_pin(TEST_C, (GPIO_DIR_INPUT | GPIO_INIT_HIGH));
	gpio_enable_pin_interrupt(TEST_A, GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(TEST_B, GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(TEST_C, GPIO_FALLING_EDGE);
	gpio_configure_pin(RESPONSE_A, (GPIO_DIR_OUTPUT | GPIO_INIT_HIGH));
	gpio_configure_pin(RESPONSE_B, (GPIO_DIR_OUTPUT | GPIO_INIT_HIGH));
    gpio_configure_pin(RESPONSE_C, (GPIO_DIR_OUTPUT | GPIO_INIT_HIGH));

    while(1){
		if ( interrupt_flag & 0x01 ) {
			gpio_clr_gpio_pin(RESPONSE_A);
			busy_delay_us(5);
			gpio_set_gpio_pin(RESPONSE_A);
			interrupt_flag &= 0x6;
		}
		if ( interrupt_flag & 0x02 ) {
			busy_delay_us(100);
			gpio_clr_gpio_pin(RESPONSE_B);
			busy_delay_us(5);
			gpio_set_gpio_pin(RESPONSE_B);
			interrupt_flag &= 0x5;
		}
		if ( interrupt_flag & 0x04 ) {
			gpio_clr_gpio_pin(RESPONSE_C);
			busy_delay_us(5);
			gpio_set_gpio_pin(RESPONSE_C);
			interrupt_flag &= 0x3;
		}
        //if ( !gpio_get_pin_value(TEST_A) ) {
			//gpio_clr_gpio_pin(RESPONSE_A);
			//busy_delay_us(5);
			//gpio_set_gpio_pin(RESPONSE_A);
		//}
		//if ( !gpio_get_pin_value(TEST_B) ) {
			//gpio_clr_gpio_pin(RESPONSE_B);
			//busy_delay_us(5);
			//gpio_set_gpio_pin(RESPONSE_B);
		//}
		//if ( !gpio_get_pin_value(TEST_C) ) {
			//gpio_clr_gpio_pin(RESPONSE_C);
			//busy_delay_us(5);
			//gpio_set_gpio_pin(RESPONSE_C);
		//}
    }
}
