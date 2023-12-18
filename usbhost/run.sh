gdb-multiarch -ex "target remote localhost:3333" -ex "load" -ex "monitor reset init" -ex "break main" -ex "continue" host_cdc_msc_hid.elf
#gdb-multiarch -ex "target remote localhost:3333" -ex "load" -ex "monitor reset init" -ex "run" host_cdc_msc_hid.elf

