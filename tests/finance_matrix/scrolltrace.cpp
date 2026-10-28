#include <QtWidgets>
#include <dlfcn.h>
#include <execinfo.h>
#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

namespace {
using Original=void(*)(QScrollArea*,QWidget*,int,int);
Original originalFunction=nullptr;
unsigned long calls=0,emptyCalls=0;
void jump(unsigned char *at,const void *destination) {
    const unsigned char indirect[]{0xff,0x25,0,0,0,0};
    std::memcpy(at,indirect,6);std::memcpy(at+6,&destination,8);
}
__attribute__((constructor)) void initializeTrace() {
    if(std::strcmp(qVersion(),"6.10.3")!=0){std::fprintf(stderr,"TRACE unsupported Qt version %s\n",qVersion());std::abort();}
    void *address=dlsym(RTLD_NEXT,"_ZN11QScrollArea19ensureWidgetVisibleEP7QWidgetii");
    if(!address){std::fprintf(stderr,"TRACE dlsym failed: %s\n",dlerror());std::abort();}
    originalFunction=reinterpret_cast<Original>(address);
    if(!std::getenv("EMCI_TRACE_INTERNAL"))return;
    // This test-only detour is necessary because Qt binds calls originating
    // inside QtWidgets directly, bypassing ordinary ELF symbol interposition.
    // Verify the exact installed x86_64 Qt 6.10.3 prologue before touching the
    // live process. The copied instructions contain no PC-relative operands.
    const unsigned char expected[]{0xf3,0x0f,0x1e,0xfa,0x41,0x57,0x41,0x56,0x41,0x55,0x49,0x89,0xf5,0x41,0x54};
    if(std::memcmp(address,expected,sizeof(expected))!=0){std::fprintf(stderr,"TRACE unsupported Qt function prologue\n");std::abort();}
    const auto pageSize=static_cast<size_t>(sysconf(_SC_PAGESIZE));
    auto *trampoline=static_cast<unsigned char*>(mmap(nullptr,pageSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if(trampoline==MAP_FAILED){std::perror("TRACE mmap");std::abort();}
    std::memcpy(trampoline,expected,sizeof(expected));jump(trampoline+sizeof(expected),static_cast<unsigned char*>(address)+sizeof(expected));
    if(mprotect(trampoline,pageSize,PROT_READ|PROT_EXEC)!=0){std::perror("TRACE trampoline mprotect");std::abort();}
    originalFunction=reinterpret_cast<Original>(trampoline);
    const auto page=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address)&~(pageSize-1));
    if(mprotect(page,pageSize,PROT_READ|PROT_WRITE|PROT_EXEC)!=0){std::perror("TRACE entry mprotect");std::abort();}
    void *handler=dlsym(RTLD_DEFAULT,"_ZN11QScrollArea19ensureWidgetVisibleEP7QWidgetii");
    if(handler==address||!handler){std::fprintf(stderr,"TRACE handler lookup failed\n");std::abort();}
    unsigned char replacement[sizeof(expected)];std::memset(replacement,0x90,sizeof(replacement));jump(replacement,handler);std::memcpy(address,replacement,sizeof(replacement));
    __builtin___clear_cache(static_cast<char*>(address),static_cast<char*>(address)+sizeof(expected));
    if(mprotect(page,pageSize,PROT_READ|PROT_EXEC)!=0){std::perror("TRACE restore entry mprotect");std::abort();}
    std::fprintf(stderr,"TRACE_HOOK installed exact Qt6.10.3 method entry=%p trampoline=%p; original function behavior retained\n",address,static_cast<void*>(trampoline));
}
__attribute__((destructor)) void finalTrace() {
    std::fprintf(stderr,"TRACE_SUMMARY calls=%lu empty=%lu\n",calls,emptyCalls);
}
}

void QScrollArea::ensureWidgetVisible(QWidget *childWidget,int xmargin,int ymargin) {
    ++calls;
    const bool empty=widget()==nullptr;
    if(empty) {
        ++emptyCalls;
        const auto receiverName=objectName().toUtf8();
        const auto childName=childWidget?childWidget->objectName().toUtf8():QByteArray("NULL");
        auto *focus=QApplication::focusWidget();
        const auto focusName=focus?focus->objectName().toUtf8():QByteArray("NULL");
        std::fprintf(stderr,"EMPTY_SCROLL_ENSURE receiver=%p object=%s child=%p childName=%s focus=%p focusName=%s\n",static_cast<void*>(this),receiverName.constData(),static_cast<void*>(childWidget),childName.constData(),static_cast<void*>(focus),focusName.constData());
        if(emptyCalls<=10){void *frames[40];const int count=backtrace(frames,40);backtrace_symbols_fd(frames,count,2);}
    }
    originalFunction(this,childWidget,xmargin,ymargin);
}
