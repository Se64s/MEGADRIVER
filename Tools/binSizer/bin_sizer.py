# bin_sizer.py

def _get_allocated_section_sizes(elffile):
    from elftools.elf.constants import SH_FLAGS

    text = data = bss = 0
    for section in elffile.iter_sections():
        sh_flags = section["sh_flags"]
        if sh_flags & SH_FLAGS.SHF_ALLOC == 0:
            continue  # Section is not in final binary

        section_size = section["sh_size"]

        if (sh_flags & SH_FLAGS.SHF_EXECINSTR != 0 or
            sh_flags & SH_FLAGS.SHF_WRITE == 0):
            text += section_size
        elif section["sh_type"] != "SHT_NOBITS":
            data += section_size
        else:
            bss += section_size

    return (text, data, bss)


def pretty_size_elf(fname, fsize, rsize):
    from elftools.elf.elffile import ELFFile

    with open(fname, "rb") as ef:
        text, data, bss = _get_allocated_section_sizes(ELFFile(ef))
    flash_use = text + data
    flash_per = flash_use / fsize
    ram_use = bss + data
    ram_per = ram_use / rsize
    print("Flash:\t%.02f KB\t(%02.02f %%)" % (flash_use / 1000, 100.0 * flash_per))
    print("Ram:\t%.02f KB\t(%02.02f %%)" % (ram_use / 1000, 100.0 * ram_per))


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 4:
        print("Please specify a path to an ELF, flash size and ram size")
        exit(-1)
    flash_size = int(sys.argv[2])
    ram_size = int(sys.argv[3])
    pretty_size_elf(sys.argv[1], flash_size, ram_size)