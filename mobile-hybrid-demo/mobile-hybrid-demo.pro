# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile
dir1.source = html
dir1.target = html
DEPLOYMENTFOLDERS = dir1

# Avoid auto screen rotation
DEFINES += ORIENTATIONLOCK

# Needs to be defined for Symbian
DEFINES += NETWORKACCESS

symbian:TARGET.UID3 = 0xA002D387

TARGET=HTML5
QT+=webkitwidgets
CONFIG += mobility
MOBILITY += sensors

# opengl

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
# CONFIG += mobility
# MOBILITY +=

SOURCES += main.cpp mainwindow.cpp  webwidget.cpp

HEADERS += mainwindow.h  webwidget.h
FORMS +=

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
