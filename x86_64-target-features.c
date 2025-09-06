#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

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

void __attribute__((target("evex512,avx512bf16")))
check_avx512_bf16(void) {
    asm volatile("vdpbf16ps %%zmm0, %%zmm1, %%zmm2" ::: "zmm0", "zmm1", "zmm2");
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
        {"+avx512bf16", check_avx512_bf16},
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
