#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <asm/prctl.h>
#endif

void __attribute__((target("avx")))
check_avx(void) {
    asm volatile("vaddps %%ymm0, %%ymm1, %%ymm2" ::: "ymm0", "ymm1", "ymm2");
}

void __attribute__((target("avx2")))
check_avx2(void) {
    asm volatile("vpaddd %%ymm0, %%ymm1, %%ymm2" ::: "ymm0", "ymm1", "ymm2");
}

void __attribute__((target("bmi2")))
check_bmi2(void) {
    asm volatile("pext %%rax, %%rbx, %%rcx" ::: "rax", "rbx", "rcx");
}

void __attribute__((target("evex512,avx512f")))
check_avx512f(void) {
    asm volatile("vaddps %%zmm0, %%zmm1, %%zmm2" ::: "zmm0", "zmm1", "zmm2");
}

void __attribute__((target("evex512,avx512vnni")))
check_avx512_vnni(void) {
    asm volatile("vpdpbusd %%zmm0, %%zmm1, %%zmm2" ::: "zmm0", "zmm1", "zmm2");
}

void __attribute__((target("evex512,avx512vbmi")))
check_avx512_vbmi(void) {
    asm volatile("vpermb %%zmm0, %%zmm1, %%zmm2" ::: "zmm0", "zmm1", "zmm2");
}

void __attribute__((target("evex512,avx512bf16")))
check_avx512_bf16(void) {
    asm volatile("vdpbf16ps %%zmm0, %%zmm1, %%zmm2" ::: "zmm0", "zmm1", "zmm2");
}

static const unsigned char
__attribute__((aligned(64)))
amx_cfg[64] = {
    [ 0] =  1,
    [16] = 64,
    [18] = 64,
    [20] = 64,
    [48] = 16,
    [49] = 16,
    [50] = 16,
};

void __attribute__((target("amx-tile")))
check_amx_tile(void) {
    asm volatile(
        "ldtilecfg %0\n\t"
        "tilerelease\n\t"
        :: "m"(amx_cfg) : "memory", "tmm2"
    );
}

void __attribute__((target("amx-int8")))
check_amx_int8(void) {
    asm volatile(
        "ldtilecfg %0\n\t"
        "tdpbssd %%tmm0, %%tmm1, %%tmm2\n\t"
        "tilerelease\n\t"
        :: "m"(amx_cfg) : "memory", "tmm0", "tmm1", "tmm2"
    );
}

void __attribute__((target("amx-bf16")))
check_amx_bf16(void) {
    asm volatile(
        "ldtilecfg %0\n\t"
        "tdpbf16ps %%tmm0, %%tmm1, %%tmm2\n\t"
        "tilerelease\n\t"
        :: "m"(amx_cfg) : "memory", "tmm0", "tmm1", "tmm2"
    );
}

static sigjmp_buf jmp;

static void
handler(int sig)
{
    (void)sig;
    siglongjmp(jmp, 1);
}

int
main(int argc, char *argv[])
{
#ifdef __linux__
    syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, 18);
#endif

    struct sigaction sa = {
        .sa_handler = handler
    };
    sigaction(SIGILL, &sa, NULL);

    const struct {
        const char *target;
        void (*fn)(void);
    } t[] = {
        {"+avx",        check_avx        },
        {"+avx2",       check_avx2       },
        {"+bmi2",       check_bmi2       },
        {"+avx512f",    check_avx512f    },
        {"+avx512vnni", check_avx512_vnni},
        {"+avx512vbmi", check_avx512_vbmi},
        {"+avx512bf16", check_avx512_bf16},
        {"+amx-tile",   check_amx_tile   },
        {"+amx-int8",   check_amx_int8   },
        {"+amx-bf16",   check_amx_bf16   },
    };
    size_t count = sizeof(t) / sizeof(t[0]);

    for (size_t i = 0; i < count; i++) {
        if (sigsetjmp(jmp, 1) == 0) {
            t[i].fn();
            printf("%s", t[i].target);
        }
    }
    printf("\n");
    return 0;
}
