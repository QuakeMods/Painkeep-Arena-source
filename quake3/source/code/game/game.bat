set LIBRARY= 
set INCLUDE= 

mkdir vm
cd vm

lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_main.c
@if errorlevel 1 goto quit

lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_syscalls.c
@if errorlevel 1 goto quit

lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../bg_misc.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../bg_lib.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../bg_pmove.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../bg_slidemove.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../q_math.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../q_shared.c
@if errorlevel 1 goto quit

lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_dmnet.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_dmq3.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_main.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_chat.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_cmd.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_team.c
@if errorlevel 1 goto quit

lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_active.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_arenas.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_bot.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_client.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_cmds.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_combat.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_items.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_mem.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_misc.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_missile.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_mover.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_session.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_spawn.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_svcmds.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_target.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_team.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_trigger.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_utils.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../g_weapon.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui  ../ai_vcmd.c
@if errorlevel 1 goto quit

q3asm -f ../game

rem copy the qvm to a staging area...
rem copy \quake3\baseq3\vm\qagame.qvm c:\workmb\pkarena31\pkarena0\vm\qagame.qvm

echo "Compilation was successful!"

:quit
cd ..
