################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-2061385439:
	@$(MAKE) --no-print-directory -Onone -f Aux_files/subdir_rules.mk build-2061385439-inproc

build-2061385439-inproc: ../Aux_files/release.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_60_02_34_core/xs" --xdcpath="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source;C:/ti/simplelink_msp432p4_sdk_3_40_01_02/kernel/tirtos/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.msp432:MSP432P401R -r debug -c "C:/ti_v12/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-2061385439 ../Aux_files/release.cfg
configPkg/compiler.opt: build-2061385439
configPkg/: build-2061385439

Aux_files/%.obj: ../Aux_files/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti_v12/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/alang/Documents/Axel/7mo semestre/Arqui/U3/Lab 1.1/INT2/HVAC_un_hilo_obj" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source/ti/posix/ccs" --include_path="C:/ti_v12/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/include" --advice:power=none -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="Aux_files/$(basename $(<F)).d_raw" --obj_directory="Aux_files" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


