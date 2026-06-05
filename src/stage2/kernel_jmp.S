.global kernel_jmp
// void kernel_jmp(void* addr);
#define MAGIC 0xB19B00B5
#define EFLAGS_ID (1 << 21)
#define CPUID_EXTENSIONS 0x80000000 // returns the maximum extended requests for cpuid
#define CPUID_EXT_FEATURES 0x80000001 // returns flags containing long mode support among other things
#define CPUID_EDX_EXT_FEAT_LM (1 << 29)
#define CR0_PAGING (1 << 31)


.extern boot_info
kernel_jmp:
    movl 4(%esp), %ecx
    push %ecx
    call test_cpuid
    call test_long_mode
    call init_paging
    call init_gdt 

    xchg %bx, %bx
    pop %ecx
    movl $MAGIC, %eax
    movl boot_info, %ebx

    ljmpl $0x8, $long_mode_trampoline



test_cpuid:
    pushfl
    pop %eax
    mov %eax, %ecx
    xor $EFLAGS_ID, %eax
    
    push %eax
    popf

    xor %eax, %ecx
    jz long_mode_error
    ret

test_long_mode:
    mov $CPUID_EXTENSIONS, %eax
    cpuid
    cmp $CPUID_EXT_FEATURES, %eax
    jb long_mode_error

    mov $CPUID_EXT_FEATURES, %eax
    cpuid
    test $CPUID_EDX_EXT_FEAT_LM, %edx
    jz long_mode_error
    ret


#define SIZEOF_PAGE_TABLE 4096
#define PML4T_ADDR 0x5000
#define PDPT_ADDR 0x2000
#define PDT_ADDR 0x3000
#define PT_ADDR 0x4000
#define PT_ADDR_MASK 0xffffffffff000
#define PT_PRESENT 1                 // marks the entry as in use
#define PT_READABLE 2                // marks the entry as r/w
#define ENTRIES_PER_PT 512
#define SIZEOF_PT_ENTRY 8
#define PAGE_SIZE 0x1000
#define CR4_PAE_ENABLE 1 << 5

#define PML4 (PDPT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE)
#define PDPT (PDT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE)
#define PDT (PT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE)
#define PT (PT_PRESENT | PT_READABLE)

#define EFER_MSR 0xC0000080
#define EFER_LM_ENABLE (1 << 8)
#define CR0_PG_ENABLE (1 << 31)
#define PHYS_BASE  0x00100000   // 1MB
#define VIRT_BASE  0xFFFFFFFF80000000  // 64-bit Higher Half (or 0xC0000000 for 32-bit)
#define ENTRIES_PER_TAB 512          // 9 bits of address per table level



init_paging:
    
    mov $2000, %edi
    xor %eax, %eax
    mov $(1024 * 4), %ecx                                    
    cld
    rep stosl

    mov $PML4T_ADDR, %edi
    mov %edi, %cr3
        
    // PML4 Setup (Upper 32-bits are safely 0 due to the clear above)
    movl $(PDPT_ADDR | PT), (%edi)                          // PML4[0]
    movl $(PDPT_ADDR | PT), (511 * SIZEOF_PT_ENTRY)(%edi)   // PML4[511] = &PDPT[510]

    mov $PDPT_ADDR, %edi
    // PDPT Setup (Upper 32-bits are safely 0 due to the clear above)
    movl $(PDT_ADDR | PT), (%edi)                           // PDPT[0]
    movl $(PDT_ADDR | PT), (510 * SIZEOF_PT_ENTRY)(%edi)    // PDPT[510]

    mov $PDT_ADDR, %edi
    // PDT Setup (Upper 32-bits are safely 0 due to the clear above)
    movl $(PT_ADDR | PT), (%edi)                            // PDT[0]

    mov $PT_ADDR, %edi                                       // Destination: first page table
    xor %eax, %eax                                           // Physical frame base starts at 0x00000000
    mov $ENTRIES_PER_PT, %ecx                                // Fill all 512 PT entries

fill_pt:
    mov %eax, %edx                                           // edx = current physical frame address
    or $PT, %edx                                             // Add present + writable flags
    
    movl %edx, (%edi)                                        // Write PTE (low 32 bits)
    movl $0, 4(%edi)                                         // Write PTE (high 32 bits = 0)
    
    add $PAGE_SIZE, %eax                                     // Next 4 KiB physical frame
    add $SIZEOF_PT_ENTRY, %edi                               // Advance to next PT slot (8-byte entry)
    loop fill_pt                                             // Repeat for 512 entries

    // Enable PAE
    mov %cr4, %eax
    or $CR4_PAE_ENABLE, %eax
    mov %eax, %cr4

    // Enable Long Mode
    mov $EFER_MSR, %ecx
    rdmsr
    or $EFER_LM_ENABLE, %eax
    wrmsr

    // Enable Paging
    mov %cr0, %eax
    or $CR0_PG_ENABLE, %eax
    mov %eax, %cr0

    ret


init_gdt:
    mov $gdt64_ptr, %eax
    lgdt (%eax)
    ret


long_mode_error:
1:  jmp 1b


.code64
long_mode_trampoline:
    mov $0x10, %dx
    mov %dx, %ds
    mov %dx, %es
    mov %dx, %fs
    mov %dx, %gs
    mov %dx, %ss

    movl %eax, %eax
    movl %ebx, %ebx
    movl %ecx, %ecx

    movl $0xFFFFFFFF, %edx
    shl $32, %rdx

    orq $-2147483648, %rdx
    

    orq %rdx, %rcx

    jmp *%rcx

.code32
.section .data

gdt64:
    .quad 0x0000000000000000 // Null descriptor
    .quad 0x00AF9A000000FFFF // Code segment descriptor (base=0, limit=4GB, flags=0x9A)
    .quad 0x00AF92000000FFFF // Data segment descriptor (base=0, limit=4GB, flags=0x92)

gdt64_ptr:
    .word 24 - 1 // Limit (size of GDT - 1)
    .long gdt64   // Base address of GDT
    .long 0         // Padding to make it 16 bytes total