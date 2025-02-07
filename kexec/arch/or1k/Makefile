#
# kexec or1k (linux booting linux)
#
include $(srcdir)/kexec/libfdt/Makefile.libfdt

or1k_KEXEC_SRCS +=  kexec/arch/or1k/kexec-or1k.c \
	kexec/arch/or1k/crashdump-or1k.c \
	kexec/arch/or1k/kexec-elf-rel-or1k.c \
	kexec/arch/or1k/kexec-elf-or1k.c

or1k_VIRT_TO_PHYS =

or1k_CPPFLAGS += -I $(srcdir)/kexec/

or1k_KEXEC_SRCS += $(libfdt_SRCS) $(or1k_libfdt_SRCS)

or1k_ASFLAGS = -Wa,--noexecstack

dist += kexec/arch/or1k/Makefile $(or1k_KEXEC_SRCS)			\
	kexec/arch/or1k/crashdump-or1k.h				\
	kexec/arch/or1k/kexec-or1k.h					\
	kexec/arch/or1k/include/arch/options.h

or1k_FS2DT += kexec/fs2dt.c
or1k_FS2DT_INCLUDE += \
	-include $(srcdir)/kexec/arch/or1k/crashdump-or1k.h \
	-include $(srcdir)/kexec/arch/or1k/kexec-or1k.h

or1k_DT_OPS += kexec/dt-ops.c

ifdef HAVE_LIBFDT

LIBS += -lfdt

else

include $(srcdir)/kexec/libfdt/Makefile.libfdt

libfdt_SRCS += $(LIBFDT_SRCS:%=kexec/libfdt/%)
or1k_CPPFLAGS += -I$(srcdir)/kexec/libfdt
or1k_KEXEC_SRCS += $(libfdt_SRCS)

endif
