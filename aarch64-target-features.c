#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

void __attribute__((target("+dotprod")))
check_dotprod(void) {
    asm volatile("sdot v0.4s, v0.16b, v0.16b" ::: "v0");
}

void __attribute__((target("+i8mm")))
check_i8mm(void) {
    asm volatile("smmla v0.4s, v0.16b, v0.16b" ::: "v0");
}

void __attribute__((target("+sve")))
check_sve(void) {
    asm volatile("incw x0" ::: "x0");
}

void __attribute__((target("+sve2")))
check_sve2(void) {
    asm volatile("smlalb z0.s, z0.h, z0.h" ::: "z0");
}

void __attribute__((target("+sve2+sme")))
check_sme(void) {
    asm volatile("smstart sm\n\tsmstop sm");
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
        const char *name;
        const char *target;
        void (*fn)(void);
    } t[] = {
        {"dotprod", "+dotprod", check_dotprod},
        {"i8mm",    "+i8mm",    check_i8mm},
        {"sve",     "+sve",     check_sve},
        {"sve2",    "+sve2",    check_sve2},
        {"sme",     "+sme",     check_sme},
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
