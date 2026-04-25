#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<VM/vm.h>
#include<VM/vm_settings.h>
#include<VM/vm_shell.h>
#include<CPU/registers.h>
#include<utils/hashmap/hmap.h>

int main(int argc, char** argv) {
    
    if (argc < 2) {
        vm_shell();
    }
    else {
        hmap* all = vm_get_settings(CONFIG_PATH);
        vm_set_settings(all, &vm_conf);
        vm_init(vm_conf.mem_size);
        
        vm_load_binary_file(vm_memory.ram, argv[1], 0x800);
        vm_boot_sequence(vm_cpu, &vm_memory);
        vm_shutdown();
    }
    return 0;
}
