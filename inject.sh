GAMEPID=$(pidof Vortex) || { echo "NO GAME"; exit 1; }

args+=(-pid $GAMEPID)
args+=(-ex "catch signal SIGSEGV SIGABRT SIGFPE SIGILL SIGBUS")
args+=(-ex "call (void*)dlopen(\"$PWD/build/libvortex-script-dumper.so\", 2)")
args+=(-ex "call (char*)dlerror()")
args+=(-ex "continue")

sudo gdb "${args[@]}"

