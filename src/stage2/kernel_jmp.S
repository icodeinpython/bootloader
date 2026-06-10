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
#define PML4T_ADDR 0x3000        /* Shifted up slightly to give tables their own 4KB spaces cleanly */
#define PDPT_ADDR  0x4000
#define PDT_ADDR   0x5000
/* We no longer need PT_ADDR (Level 1) because we are using 2MB Huge Pages! */

#define PT_PRESENT   (1 << 0)
#define PT_READABLE  (1 << 1)
#define PT_HUGE_PAGE (1 << 7)    /* Bit 7 in PDT entry makes it a 2MB page */

#define EFER_MSR       0xC0000080
#define EFER_LM_ENABLE (1 << 8)
#define CR4_PAE_ENABLE (1 << 5)
#define CR0_PG_ENABLE  (1 << 31)

init_paging:
    /* ---------------------------------------------------------------------
     * 1. Clear Page Table Memory Space (0x3000 to 0x5000)
     * --------------------------------------------------------------------- */
    mov $0x3000, %edi
    xor %eax, %eax
    mov $(1024 * 3), %ecx        /* Clearing 3 tables * 4096 bytes = 12288 bytes (3072 dwords) */
    cld
    rep stosl

    /* Set CR3 to point to the PML4 table */
    mov $PML4T_ADDR, %edi
    mov %edi, %cr3
        
    /* ---------------------------------------------------------------------
     * 2. PML4 Setup (Level 4)
     * --------------------------------------------------------------------- */
    /* Entry 0: Identity mapping (0x0000000000000000) */
    movl $(PDPT_ADDR | PT_PRESENT | PT_READABLE), (%edi)
    
    /* Entry 256: Direct Map Window (0xFFFF800000000000) */
    movl $(PDPT_ADDR | PT_PRESENT | PT_READABLE), (256 * 8)(%edi)

    /* Entry 511: Kernel Executable Base (0xFFFFFFFF80000000) */
    movl $(PDPT_ADDR | PT_PRESENT | PT_READABLE), (511 * 8)(%edi)

    /* ---------------------------------------------------------------------
     * 3. PDPT Setup (Level 3)
     * --------------------------------------------------------------------- */
    mov $PDPT_ADDR, %edi
    
    /* Entry 0: Maps virtual 0G-512G (Covers Identity and Direct Map Entry 256) */
    movl $(PDT_ADDR | PT_PRESENT | PT_READABLE), (%edi)                           
    
    /* Entry 510: Maps virtual 510G-511G (Covers Kernel Base Entry 511) */
    movl $(PDT_ADDR | PT_PRESENT | PT_READABLE), (510 * 8)(%edi)

    /* ---------------------------------------------------------------------
     * 4. PDT Setup (Level 2) - Map 1 GiB of RAM using 512 x 2 MiB Huge Pages
     * --------------------------------------------------------------------- */
    mov $PDT_ADDR, %edi
    xor %eax, %eax               /* Physical address starts at 0x00000000 */
    mov $512, %ecx               /* 512 entries * 2MB = 1024MB (1 GiB) */

fill_pdt:
    mov %eax, %edx
    or $(PT_PRESENT | PT_READABLE | PT_HUGE_PAGE), %edx   /* Notice PT_HUGE_PAGE flag */
    
    movl %edx, (%edi)            /* Write low 32 bits of PDT entry */
    movl $0, 4(%edi)             /* Write high 32 bits (0 for first 4GB of RAM) */
    
    add $0x200000, %eax          /* Move physical address forward by 2 MiB (0x200000) */
    add $8, %edi                 /* Move to next PDT table slot */
    loop fill_pdt

    /* ---------------------------------------------------------------------
     * 5. CPU Register Activation
     * --------------------------------------------------------------------- */
    /* Enable PAE */
    mov %cr4, %eax
    or $CR4_PAE_ENABLE, %eax
    mov %eax, %cr4

    /* Enable Long Mode (EFER) */
    mov $EFER_MSR, %ecx
    rdmsr
    or $EFER_LM_ENABLE, %eax
    wrmsr

    /* Enable Paging (CR0) */
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