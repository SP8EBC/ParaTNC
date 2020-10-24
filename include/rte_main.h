
#ifndef RTE_MAIN_H_
#define RTE_MAIN_H_

#include <stdint.h>

extern uint8_t rte_main_reboot_req;

extern uint8_t rte_main_boot_cycles, rte_main_hard_faults;
extern uint32_t rte_main_hardfault_lr, rte_main_hardfault_pc;

extern uint8_t rte_main_trigger_status;
extern uint8_t rte_main_trigger_modbus_status;


#endif
