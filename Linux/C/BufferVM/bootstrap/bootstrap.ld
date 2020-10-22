SECTIONS
{
        .bootloader 0 : {
                 *bootloader.s.o(.text)
        }
}