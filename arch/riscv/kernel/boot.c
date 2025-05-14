/* RISC-V kernel boot stage */
#include <pgtable.h>
#include <asm.h>

#define ARRTIBUTE_BOOTKERNEL __attribute__((section(".bootkernel")))

typedef void (*kernel_entry_t)(unsigned long);

/********* setup memory mapping ***********/
static uintptr_t ARRTIBUTE_BOOTKERNEL alloc_page()
{
    static uintptr_t pg_base = PGDIR_PA;
    pg_base += 0x1000;
    return pg_base;
}

// using 2MB large page
static void ARRTIBUTE_BOOTKERNEL map_page(uint64_t va, uint64_t pa, PTE *pgdir)
{
    va &= VA_MASK;
    uint64_t vpn2 =
        va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
    uint64_t vpn1 = (vpn2 << PPN_BITS) ^
                    (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
    if (pgdir[vpn2] == 0) {
        // alloc a new second-level page directory
        set_pfn(&pgdir[vpn2], alloc_page() >> NORMAL_PAGE_SHIFT);
        set_attribute(&pgdir[vpn2], _PAGE_PRESENT);
        clear_pgdir(get_pa(pgdir[vpn2]));
    }
    PTE *pmd = (PTE *)get_pa(pgdir[vpn2]);
    set_pfn(&pmd[vpn1], pa >> NORMAL_PAGE_SHIFT);
    set_attribute(
        &pmd[vpn1], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
                        _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY);
}

static void ARRTIBUTE_BOOTKERNEL enable_vm()
{
    // write satp to enable paging
    set_satp(SATP_MODE_SV39, 0, PGDIR_PA >> NORMAL_PAGE_SHIFT);
    local_flush_tlb_all();
}

/* Sv-39 mode
 * 0x0000_0000_0000_0000-0x0000_003f_ffff_ffff is for user mode
 * 0xffff_ffc0_0000_0000-0xffff_ffff_ffff_ffff is for kernel mode
 */
static void ARRTIBUTE_BOOTKERNEL setup_vm()
{
    clear_pgdir(PGDIR_PA);
    // map kernel virtual address(kva) to kernel physical
    // address(kpa) kva = kpa + 0xffff_ffc0_0000_0000 use 2MB page,
    // map all physical memory
    PTE *early_pgdir = (PTE *)PGDIR_PA;
    for (uint64_t kva = 0xffffffc050000000lu;
         kva < 0xffffffc060000000lu; kva += 0x200000lu) {
        map_page(kva, kva2pa(kva), early_pgdir);
    }
    // map boot address
    for (uint64_t pa = 0x50000000lu; pa < 0x51000000lu;
         pa += 0x200000lu) {
        map_page(pa, pa, early_pgdir);
    }
    enable_vm();
}

extern uintptr_t _start[];

/*********** start here **************/
int ARRTIBUTE_BOOTKERNEL boot_kernel(unsigned long mhartid)
{
    if (mhartid == 0) {
        setup_vm();
    } else {
        enable_vm();
    }

    /* enter kernel */
    ((kernel_entry_t)pa2kva(_start))(mhartid);

    return 0;
}
