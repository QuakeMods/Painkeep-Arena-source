---------------------------------------------------------------------------
---------------------------------------------------------------------------
PainKeep Arena 3.0n Source Release 
Release Date: September 27,2007

Materials in this release are based on the 
PainKeep Arena 3.0n Full Release - March 19, 2004
Copyright (c)2007-2008 Team Evolve.
All rights reserved.
---------------------------------------------------------------------------
---------------------------------------------------------------------------

Team Evolve:
Team Evolve Web Site - http://www.teamevolve.com

Community Information:
http://www.groundplan.com/gplan_forum

---------------------------------------------------------------------------

Team Evolve welcomes you to the source files of PainKeep Arena! We release this information in the hopes that it will contribute to the gaming community with a jumpstart into the next generation of online games. Should you use any information within the source files toward your game then please give credit to Team Evolve in the form of a textual reference in the public readme file.

Team Evolve releases the source in an AS-IS form and will not support nor answer any questions into its workings. Team Evolve will not support nor be responsible in any way to extensions made to Painkeep Arena from third party modifications to source or content.

---------------------------------------------------------------------------

PainKeep Arena is based on the 1.32 version of Quake III Arena. IMPORTANT: In order to use any modification to the  PainKeep Arena source code will require: 1) Quake III Arena 1.32 installed and 2) the Full Install of PainKeep Arena 3.0n.

This help document is based on Windows XP SP2. If you use another OS then you will need to adjust search PATH information accordingly.

The PainKeep Arena source code is based on the GAME Source released by id Software. PainKeep Arena source must be installed into the same place as the id software GAME Source, C:\QUAKE3, in order for the lcc and q3cpp compilation paths to be correct.

You must place the compilation executables in your system’s path. The lcc executable is located in c:\quake3\bin_nt directory. Add to your systems path using the Control Panel’s System tool, select the Advanced Tab and then click on the “Environment Variables” Button. Under the “Systems Variables”, select “Path” and Click the “Edit” button to add to the end of the existing path line - “;c:\quake3\bin_nt” - {without the quotes, of course}. Click OK a few time to save the new Path setting. You may need to reboot Windows for new Path setting to be recognized by the system.

The PainKeep Arena source code contains the procedures and logic that govern the User Interface, Game Logic, Client Presentation. Each section is primarily controlled by a specific directory within the source code… 
User Interface - c:\quake3\source\code\ui\
Game Logic -  c:\quake3\source\code\game\
Client Presentation - c:\quake3\source\code\cgame\

Please note that some modifications to the User Interface will require changes in Client Presentation area.

PainKeep Arena uses QVM files to hold the compiled game code. Each section (User Interface, Game Logic, Client Presentation) has an associated and separate QVM file. To create, (or compile), a new QVM, each section have a .BAT file that can be used to generate a new QVM. 

To compile a new…
User Interface:
	execute c:\quake3\source\code\ui\ui.bat
Game Logic:
	execute c:\quake3\source\code\game\game.bat
Client Presentation:
	execute c:\quake3\source\code\cgame\cgame.bat
  
The GAME and CLIENT QVM files will be generated in directory: 
	c:\quake3\baseq3\vm\
	gagame.qvm
	cgame.qvm

The USER INTERFACE QVM file will be generated in directory: 
	c:\quake3\missionpack\vm\
	ui.qvm

After you make your QVM files you will need to make a .PK3 file that holds your newly generated QVMs and any other Modification materials. Please note that .PK3 files can be managed by ZIP tools. 

Create a .PK3 file with the new QVM files in a \vm subdirectory.

Place the newly generated .PK3 file into your mod (or \PKARENA) directory under the Quake III Arena Execution directory. Reminder, Quake III Arena will use the .PK3 File’s material that comes last alphabetically. For example if you make a simple MOD for PainKeep Arena, place your materials in…
	..\Quake III Arena\pkarena\zpktext.pk3
	(in this example zpktest.pk3 will be loaded last and your updates will act as the effective .QVM files) 

Good Luck!
Yours, Team Evolve and Ergodic

---------------------------------------------------------------------------
---------------------------------------------------------------------------

THE LEGAL CRAP
-------------------


A. Copyright Notice
This production in its entirety and all derivative works are copyright (c)2007-2008 Team Evolve. All rights reserved.




Ownership of all new components, including, but not limited to; source code, compiled code, graphics, textures, sounds, models and maps, remain with Team Evolve and the individual authors respectively. Some components are the exclusive property of their authors and owners and are used with kind permission.
All original components are copyright (c)2001, iD Software.
Quake III Arena and the stylized 'Q' are trademarks of iD Software. 



All other trademarks are property of their respective owners and are hereby acknowledged.


B. Distribution and Usage Permissions
Team Evolve grants to the final end user an exclusive right to use this production for the purposes of personal entertainment only. Team Evolve grants to the final end user an exclusive right to freely distribute this production in its undisturbed and unaltered entirety provided no exchange, monetary or otherwise, is requested. All other media entities are expressly excluded from this right prior to acknowledge and consent from Team Evolve or one of Team Evolve's duly appointed representatives, agents or subsidiaries.

By using this product you agree to exempt, without reservation, the authors and 
owners of this production or components thereof from any responsibility for liability, damage caused, or loss, directly or indirectly, by this software, including but not limited to, any interruptions of service, loss of business, or any other consequential damages resulting from the use of or operation of this product or components thereof. 

No warranties are made, expressed or implied, regarding the usage, functionality, or implied operability of this product. All elements are available solely on an 
"as-is" basis. Usage is subject to the user's own risk.

New or altered source code components are included with kind permission of the respective authors and owners and are provided with the only intention of facilitating in the integration of this production, or components thereof, with other such freely available and non-commercial productions. Authors are expressly forbidden to use these components, or any other component of this production, as a basis for other commercially available works or demonstration systems without prior acknowledge and consent from Team Evolve or one of Team Evolve's duly appointed representatives, agents or subsidiaries.
-------------------------------------------------------------------------------------

PainKeep Arena is RATED M: Mature Audiences Only

