#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/5be4ea42/main.o \
	${OBJECTDIR}/_ext/af191497/functions.o \
	${OBJECTDIR}/_ext/422f8e28/image.o \
	${OBJECTDIR}/_ext/422f8e28/message.o \
	${OBJECTDIR}/_ext/422f8e28/monitor.o \
	${OBJECTDIR}/_ext/422f8e28/robot.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/robot

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/robot: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/robot ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/5be4ea42/main.o: ../../Documents/Tps\ Reel/superviseur_robot/destijl_init/main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5be4ea42
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5be4ea42/main.o ../../Documents/Tps\ Reel/superviseur_robot/destijl_init/main.cpp

${OBJECTDIR}/_ext/af191497/functions.o: ../../Documents/Tps\ Reel/superviseur_robot/destijl_init/src/functions.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/af191497
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/af191497/functions.o ../../Documents/Tps\ Reel/superviseur_robot/destijl_init/src/functions.cpp

${OBJECTDIR}/_ext/422f8e28/image.o: ../../Documents/Tps\ Reel/superviseur_robot/src/image.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/422f8e28
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/422f8e28/image.o ../../Documents/Tps\ Reel/superviseur_robot/src/image.cpp

${OBJECTDIR}/_ext/422f8e28/message.o: ../../Documents/Tps\ Reel/superviseur_robot/src/message.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/422f8e28
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/422f8e28/message.o ../../Documents/Tps\ Reel/superviseur_robot/src/message.cpp

${OBJECTDIR}/_ext/422f8e28/monitor.o: ../../Documents/Tps\ Reel/superviseur_robot/src/monitor.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/422f8e28
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/422f8e28/monitor.o ../../Documents/Tps\ Reel/superviseur_robot/src/monitor.cpp

${OBJECTDIR}/_ext/422f8e28/robot.o: ../../Documents/Tps\ Reel/superviseur_robot/src/robot.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/422f8e28
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/422f8e28/robot.o ../../Documents/Tps\ Reel/superviseur_robot/src/robot.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
