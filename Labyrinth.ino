#ifndef Labyrinth_H
#define Labyrinth_H

//hosts master ui things and whatever fuck you

/*
explaining what this is for:

this is more or less a file full of all the window structures, programs, and so forth for the default watch features so i don't scatter them about in a million places in the project
*/

//tasks: a group of things that are system wide dependant and hence called in main and set up by default, do not go thorugh here
//eg: updating screen periodically (can be influenced by proscesses mind you) and sensors (which set variables, tasks do things based on said variables-but the update rate can be jimmied a little by tasks.this is important for the step counter for example)






//unordered default structures for some structures


//GENERAL SCREENS--------------------------------------------------------------------------------------------------------------------------
//HOME,LOCK,BOOT

//lock screen
//based off default grouping in tthe lillypad renderer originally

//home screen
//has sets of applests with a small time window. scroolbox so that you can hit down button to see next set of apps sorted alphabetically



//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

//apps:

//clock app:
//shows all the functionality inside of the clock inside here
//timers,alarms,stopwatch
//some of these need to stsay running when the appp isn't open, so i'll have to make it run in the background wihtout the window grouping when it's closed
//ui style blue
//callendar in there

//health monitor
//heart rate stuff,sleep, steps, excersize,etc. red ui style i'll have to make. dim red background colors and shit

//files and file explorer

//remote controll
//infared

//radiofrequency stuff
//use the cc1101 to do... things i guess

//card/nfc reader

//phone link

//utils/settings

//little drop down menu for quick stuff, like the slide down menu on android

//terminal

//super rubberducky+remote m+kb

#endif