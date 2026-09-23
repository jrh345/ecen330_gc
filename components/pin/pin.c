#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

// TODO: GPIO Matrix Registers - GPIO_OUT_REG, GPIO_OUT_W1TS_REG, ...
// NOTE: Remember to enclose the macro values in parenthesis, as below
#define GPIO_OUT_REG          (DR_REG_GPIO_BASE+0x04)
#define GPIO_OUT_W1TS_REG     (DR_REG_GPIO_BASE+0x08)
#define GPIO_OUT_W1TC_REG     (DR_REG_GPIO_BASE+0x0C)
#define GPIO_OUT1_REG         (DR_REG_GPIO_BASE+0x10)
#define GPIO_OUT1_W1TS_REG    (DR_REG_GPIO_BASE+0x14)
#define GPIO_OUT1_W1TC_REG    (DR_REG_GPIO_BASE+0x18)
#define GPIO_ENABLE_REG       (DR_REG_GPIO_BASE+0x20)
#define GPIO_ENABLE_W1TS_REG  (DR_REG_GPIO_BASE+0x24)
#define GPIO_ENABLE_W1TC_REG  (DR_REG_GPIO_BASE+0x28)
#define GPIO_ENABLE1_REG      (DR_REG_GPIO_BASE+0x2C)
#define GPIO_ENABLE1_W1TS_REG (DR_REG_GPIO_BASE+0x30)
#define GPIO_ENABLE1_W1TC_REG (DR_REG_GPIO_BASE+0x34)
#define GPIO_IN_REG           (DR_REG_GPIO_BASE+0x3C)
#define GPIO_IN1_REG          (DR_REG_GPIO_BASE+0x40)
#define GPIO_FUNC_OUT_SEL_CFG_REG(n) (DR_REG_GPIO_BASE + 0x530 + (0x4 * n))
#define GPIO_PIN_REG(n)	(DR_REG_GPIO_BASE + 0x88 + (0x4 * n))
// TODO: IO MUX Registers
// HINT: Add DR_REG_IO_MUX_BASE with PIN_MUX_REG_OFFSET[n]
#define IO_MUX_REG(n)  ((DR_REG_IO_MUX_BASE)+(PIN_MUX_REG_OFFSET[n]))
// TODO: IO MUX Register Fields - FUN_WPD, FUN_WPU, ...
#define MCU_OE	 0
#define SLP_SEL  1
#define MCU_WPD  2
#define MCU_WPU  3
#define MCU_IE   4
#define MCU_DRV5 5
#define MCU_DRV6 6
#define FUN_WPD  7
#define FUN_WPU	 8
#define FUN_IE	 9
#define FUN_DRV10 10
#define FUN_DRV11 11
#define MCU_SEL12 12
#define MCU_SEL13 13
#define MCU_SEL14 14

#define REG(r) (*(volatile uint32_t *)(r))
#define REG_BITS 32
#define REG_SET_BIT(r,b)  (REG(r) |= (1 << b))
#define REG_CLR_BIT(r,b) (REG(r) &= ~(1 << b))
#define REG_GET_BIT(r,b) ((REG(r) >> b) & 0x01)

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};


// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
// Return zero if successful, or non-zero otherwise.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	}
	REG(GPIO_PIN_REG(pin)) = 0;

	REG(GPIO_FUNC_OUT_SEL_CFG_REG(pin)) = 0x100;

	// TODO: Reset IO_MUX_x_REG: MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
	REG(IO_MUX_REG(pin)) =
    (REG(IO_MUX_REG(pin)) & ~((7 << MCU_SEL12) | (3 << FUN_DRV10) | (1 << FUN_WPU))) | ((2 << MCU_SEL12) | (2 << FUN_DRV10) | (1 << FUN_WPU));
	// NOTE: By default, pin should not float, save power with FUN_WPU=1

	// Now that the pin is reset, set the output level to zero
	return pin_set_level(pin, 0);
}

// Enable or disable a pull-up on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	if (enable) REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	else REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
	return 0;
}

// Enable or disable a pull-down on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	if (enable) REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
	else REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);
	return 0;
}

// Enable or disable the pin as an input signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_input(pin_num_t pin, bool enable)
{
	if (enable) REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
	else REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);
	return 0;
}

// Enable or disable the pin as an output signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_output(pin_num_t pin, bool enable)
{
	if (pin < 32) {
		if (enable) REG_SET_BIT(GPIO_ENABLE_REG, pin);
		else REG_CLR_BIT(GPIO_ENABLE_REG, pin);
	} else {
		if (enable) REG_SET_BIT(GPIO_ENABLE1_REG, (pin - 32));
		else REG_CLR_BIT(GPIO_ENABLE1_REG, (pin - 32));
	}
	return 0;
}

// Enable or disable the pin as an open-drain signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	if (enable) REG_SET_BIT(GPIO_PIN_REG(pin), 2);
	else REG_CLR_BIT(GPIO_PIN_REG(pin), 2);
	return 0;
}

// Sets the output signal level if the pin is configured as an output.
// Return zero if successful, or non-zero otherwise.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	if (pin < 32) {
		if (level) REG_SET_BIT(GPIO_OUT_W1TS_REG, pin);
		else REG_SET_BIT(GPIO_OUT_W1TC_REG, pin);
	} else {
		if (level) REG_SET_BIT(GPIO_OUT1_W1TS_REG, (pin - 32));
		else REG_SET_BIT(GPIO_OUT1_W1TC_REG, (pin - 32));
	}
	return 0;
}

// Gets the input signal level if the pin is configured as an input.
// Return zero or one if successful, or negative otherwise.
int32_t pin_get_level(pin_num_t pin)
{
	if (pin < 32) return REG_GET_BIT(GPIO_IN_REG, pin);
	return REG_GET_BIT(GPIO_IN1_REG, (pin - 32));
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
	return (((uint64_t)REG(GPIO_IN1_REG) & 0xFFu) << 32) | REG(GPIO_IN_REG);
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
	return (((uint64_t)REG(GPIO_OUT1_REG) & 0xFFu) << 32) | REG(GPIO_OUT_REG);
}
