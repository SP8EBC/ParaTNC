#include <drivers/gpio_conf_stm32f1x.h>

void Configure_GPIO(GPIO_TypeDef* gpio, uint8_t pin, uint8_t conf){
    volatile uint32_t *crPointer;
 
    if(pin < 8)
        crPointer = &gpio->CRL;
    else {
        crPointer = &gpio->CRH;
        pin -= 8;
    }
 
    *crPointer &= ~( 0xF << (pin * 4));
    *crPointer |= conf << (pin * 4);
}
