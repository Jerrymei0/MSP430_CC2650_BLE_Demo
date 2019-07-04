################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
AP.obj: ../AP.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/bin/cl430" -vmspx --data_model=restricted --code_model=large --near_data=none -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/include" --include_path="C:/My_Doc/ccsv6/workspace/MSP430_CC2650_BLE_Demo/driverlib/MSP430F5xx_6xx" --advice:power=all --define=__MSP430F5529__ --display_error_number --diag_wrap=off --diag_warning=225 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal --preproc_with_compile --preproc_dependency="AP.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

AP_BLE.obj: ../AP_BLE.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/bin/cl430" -vmspx --data_model=restricted --code_model=large --near_data=none -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/include" --include_path="C:/My_Doc/ccsv6/workspace/MSP430_CC2650_BLE_Demo/driverlib/MSP430F5xx_6xx" --advice:power=all --define=__MSP430F5529__ --display_error_number --diag_wrap=off --diag_warning=225 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal --preproc_with_compile --preproc_dependency="AP_BLE.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

UART0.obj: ../UART0.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/bin/cl430" -vmspx --data_model=restricted --code_model=large --near_data=none -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/include" --include_path="C:/My_Doc/ccsv6/workspace/MSP430_CC2650_BLE_Demo/driverlib/MSP430F5xx_6xx" --advice:power=all --define=__MSP430F5529__ --display_error_number --diag_wrap=off --diag_warning=225 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal --preproc_with_compile --preproc_dependency="UART0.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

UART1.obj: ../UART1.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/bin/cl430" -vmspx --data_model=restricted --code_model=large --near_data=none -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/include" --include_path="C:/My_Doc/ccsv6/workspace/MSP430_CC2650_BLE_Demo/driverlib/MSP430F5xx_6xx" --advice:power=all --define=__MSP430F5529__ --display_error_number --diag_wrap=off --diag_warning=225 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal --preproc_with_compile --preproc_dependency="UART1.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/bin/cl430" -vmspx --data_model=restricted --code_model=large --near_data=none -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/include" --include_path="C:/My_Doc/ccsv6/workspace/MSP430_CC2650_BLE_Demo/driverlib/MSP430F5xx_6xx" --advice:power=all --define=__MSP430F5529__ --display_error_number --diag_wrap=off --diag_warning=225 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

myClockWithCrystals.obj: ../myClockWithCrystals.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/bin/cl430" -vmspx --data_model=restricted --code_model=large --near_data=none -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/ccsv6/tools/compiler/ti-cgt-msp430_15.12.7.LTS/include" --include_path="C:/My_Doc/ccsv6/workspace/MSP430_CC2650_BLE_Demo/driverlib/MSP430F5xx_6xx" --advice:power=all --define=__MSP430F5529__ --display_error_number --diag_wrap=off --diag_warning=225 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal --preproc_with_compile --preproc_dependency="myClockWithCrystals.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


