Return to Castle Wolfensteintm

Source Code - Readme

This code may only be used under the terms specified in the license.txt file.  No support will be provided by id, Gray Matter, Nerve, or Activision.  However, help is available.  We suggest you investigate the various user communities available through links at www.CastleWolfenstein.com


During development of Return to Castle Wolfenstein, we split the code between single and multiplayer to allow specialization.  Although very similar overall, there are specific differences between the code.  Both the source for the SP and MP code is provided here.

Changes (adding or removing functions) in the game-side of single player code will require a pre-build process, “Extractfuncs.bat”.  This is run automatically when compiling, and will generate two header files (g_funcs.h and g_func_decs.h), which are used by the game for savegame functionality.  If game code has changed, two ‘builds’ will be necessary; one to generate the new header files, then another to complete the compilation of the game.  Because these files are updated automatically, they should remain writable (not locked, or “read only”).  If this automatic process appears to not want to compile and the compiler refers to either of these header files, or “g_save.obj”, first manually delete the header files, then run extractfuncs.bat from the command line or double-click it in windows.  This happens rarely, and this procedure should always remedy the problem.
