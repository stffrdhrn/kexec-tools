#ifndef KEXEC_ARCH_OR1K_OPTIONS_H
#define KEXEC_ARCH_OR1K_OPTIONS_H


/* All 'local' loader options: */
#define OPT_APPEND      	((OPT_MAX)+0)
#define OPT_DTB         	((OPT_MAX)+1)
#define OPT_INITRD		((OPT_MAX)+2)
#define OPT_REUSE_CMDLINE	((OPT_MAX)+3)
#define OPT_ARCH_MAX		((OPT_MAX)+05)

#define	KEXEC_ARCH_OPTIONS \
	KEXEC_OPTIONS \
	{"command-line",   1, 0, OPT_APPEND}, \
	{"append",	   1, 0, OPT_APPEND}, \
	{"initrd",	   1, 0, OPT_INITRD}, \
	{"dtb",	           1, 0, OPT_DTB}, \
	{ "reuse-cmdline", 0, NULL, OPT_REUSE_CMDLINE }, \

#define KEXEC_ARCH_OPT_STR KEXEC_OPT_STR /* Only accept long arch options. */
#define KEXEC_ALL_OPTIONS KEXEC_ARCH_OPTIONS
#define KEXEC_ALL_OPT_STR KEXEC_ARCH_OPT_STR

static const char or1k_opts_usage[] __attribute__ ((unused)) =
"     --command-line=STRING Set the kernel command line to STRING.\n"
"     --append=STRING       Set the kernel command line to STRING.\n"
"     --dtb=FILE            Use FILE as the device tree blob.\n"
"     --initrd=FILE         Use FILE as the kernel initial ramdisk.\n"
"     --reuse-cmdline       Use kernel command line from running system.\n";

struct or1k_opts {
	const char *command_line;
	const char *dtb;
	const char *initrd;
};

extern struct or1k_opts or1k_opts;

#endif /* KEXEC_ARCH_OR1K_OPTIONS_H */
