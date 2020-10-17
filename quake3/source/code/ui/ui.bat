rem set PATH=%PATH%; c:\quake3\bin_nt

set LIBRARY= 
set INCLUDE= 

mkdir vm
cd vm

lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_main.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_misc.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_lib.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/q_math.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/q_shared.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_atoms.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_players.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_util.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_shared.c
@if errorlevel 1 goto quit
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_gameinfo.c
@if errorlevel 1 goto quit

q3asm -f ../ui

rem copy the qvm to a staging area...
rem copy \quake3\missionpack\vm\ui.qvm c:\workmb\pkarena31\pkarena0\vm\ui.qvm

echo "Compilation was successful!"
:quit
cd ..
