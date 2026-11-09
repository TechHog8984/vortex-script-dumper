#include <cstdio>
#include <fstream>
#include <memory>
#include <thread>
#include <link.h>

#include "polyhook2/Detour/NatDetour.hpp"
#include "polyhook2/Misc.hpp"

struct CFunctionHook {
    uint64_t trampoline = 0;
    std::unique_ptr<PLH::x64Detour> detour;
};
CFunctionHook luau_load_hook;

// namespace OFFSETS {
//     uintptr_t luau_load = 0x6a2b220; // 0.9.6
// };
namespace PATTERNS {
    constexpr const char *luau_load =
        "f3 0f 1e fa 41 57 49 89 ff 41 56 41 55 41 54 49 89 f4 55 53 48 89 d3 48 "
        "81 ec a8 01 00 00 64 48";
}

size_t script_counter = 0;
typedef int32_t (*luau_load_t)(uintptr_t L, const char *chunkname, const char* data, size_t size, int32_t env);
int32_t hooked_luau_load(uintptr_t L, const char *chunkname, const char *data, size_t size, int32_t env) {
    printf("[vsd.hooked_luau_load] luau_load called with %zu bytes\n", size);

    char filename[128];
    snprintf(filename, 128, "/tmp/vsd/script%zu.bin", script_counter++);
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file << std::string(data, size);
    file.close();

    return reinterpret_cast<luau_load_t>(luau_load_hook.trampoline)(L, chunkname, data, size, env);
}

void init() {
    puts("[vsd] initialization started.");

    std::filesystem::create_directory("/tmp/vsd");

    puts("[vsd] getting application base...");
    uintptr_t application_base = 0;
    dl_iterate_phdr([] (dl_phdr_info* info, size_t, void* data) -> int {
        *static_cast<uintptr_t*>(data) = info->dlpi_addr;
        return 1;
    }, &application_base);
    if (application_base == 0) {
        puts("[vsd] failed to get application base. quiting...");
        return;
    }
    printf("[vsd] application base: 0x%lx\n", application_base);

    puts("[vsd] scanning for luau_load...");

    uint64_t luau_load_scan = PLH::findPattern(application_base, -1, PATTERNS::luau_load);
    if (luau_load_scan == 0) {
        puts("[vsd] failed to find luau_load. quiting...");
        return;
    }
    printf("[vsd] luau_load: 0x%lx\n", luau_load_scan);

    // auto luau_load = reinterpret_cast<luau_load_t>(application_base + OFFSETS::luau_load);
    auto luau_load = reinterpret_cast<luau_load_t>(luau_load_scan);

    luau_load_hook.detour = std::make_unique<PLH::NatDetour>(reinterpret_cast<uintptr_t>(luau_load), reinterpret_cast<uintptr_t>(&hooked_luau_load), &luau_load_hook.trampoline);

    puts("[vsd] hooking luau_load...");
    if (!luau_load_hook.detour->hook()) {
        puts("[vsd] failed to hook luau. quiting.");
        return;
    }
    puts("[vsd] luau_load has been hooked.");

    puts("[vsd] initialization finished.");
}

__attribute((constructor)) void __entry()
{
    std::thread(init).detach();
}

__attribute((destructor)) void __cleanup() {
    puts("[vsd] cleaning up...");

    luau_load_hook.detour.reset();

    puts("[vsd] clean up finished.");
}

