========================================================================
    CONSOLE APPLICATION : ConsoleApplication1 Project Overview
========================================================================

AppWizard has created this ConsoleApplication1 application for you.

This file contains a summary of what you will find in each of the files that
make up your ConsoleApplication1 application.


ConsoleApplication1.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ConsoleApplication1.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

main.cpp
    This is the main application source file. Includes setting up of the environment and the hand
	detector and pose recogniser objects.

environment.cpp
	Contains the implementations for all the functions needed to create the 3D and 2D environment, and 
	any functions that deal with interaction with the cubes.

environment.h
	Header file for the environment

ml.cpp
	The functions for taking in the image features and eassifying the pose.

ml.h
	Header file for machine learning.

DepthBasics2.cpp
	Code for getting the depth image directly from the Kinect. Adapted from the code provided in the
	Microsoft Kinet SDK. Functions to locate the hands and extracts the relevant feature data.

DepthBasics2.cpp
	Header file for the hand detection.

train.cpp
	Separate main file for use in training data, and just showing had data and classified pose.

loadShader.cpp,shaders/fragmentshader.txt,shaders/vertexshader.txt
	Files to load the shaders, and containing information about the shaders




/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named ConsoleApplication1.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

Libraries used:
	- OpenCV for image processing and displaying the image
	- OpenGL for the graphics fo the 3D and 2D environment
	- Shark for the machine learning: http://image.diku.dk/shark/
	- Bullet Physics for the physics engine: http://bulletphysics.org/wordpress/

All development has been done in visual studio, I havent tested it otherwise.
For Bullet physics, need to include 3 of the individual libraries for it to work. Follow: http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Creating_a_project_from_scratch
With Shark, I performed a full installation on my machine, as opposed to linking to the build files after following the installation instructions here: http://image.diku.dk/shark/sphinx_pages/build/html/rest_sources/getting_started/installation.html
	This was not the easiest way to do it, and since all that is required to use it is to include the shark.h header, probably didn't need to be done this way

Need to link a number of external libraries, as well as the include directories in order to build and run in Visual Studio.

/////////////////////////////////////////////////////////////////////////////
