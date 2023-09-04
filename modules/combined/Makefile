###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project
# MODULE_DIR       - Location of the MOOSE modules directory
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
MODULE_DIR         ?= $(shell dirname `pwd`)
MOOSE_DIR          ?= $(shell dirname $(MODULE_DIR))
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
ADDITIONAL_CPPFLAGS += -Wall -Wextra
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ################################
ALL_MODULES       := yes
################################## MODULES ####################################
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

# CHEMICAL_REACTIONS          := yes
# CONTACT                     := yes
# ELECTROMAGNETICS            := yes
# EXTERNAL_PETSC_SOLVER       := yes
# FLUID_PROPERTIES            := yes
# FSI                         := yes
# FUNCTIONAL_EXPANSION_TOOLS  := yes
# GEOCHEMISTRY                := yes
# HEAT_CONDUCTION             := yes
# LEVEL_SET                   := yes
# MISC                        := yes
# NAVIER_STOKES               := yes
# OPTIMIZATION                := yes
# PERIDYNAMICS                := yes
# PHASE_FIELD                 := yes
# POROUS_FLOW                 := yes
# RAY_TRACING                 := yes
# REACTOR                     := yes
# RDG                         := yes
# RICHARDS                    := yes
# STOCHASTIC_TOOLS            := yes
# THERMAL_HYDRAULICS          := yes
# TENSOR_MECHANICS            := yes
# XFEM                        := yes
INCLUDE_COMBINED  := no
USE_TEST_LIBS     := yes
SKIP_LOADER       := yes
include $(MODULE_DIR)/modules.mk
###############################################################################

# Build the Module executable
APPLICATION_DIR    := $(MODULE_DIR)/combined
APPLICATION_NAME   := combined
BUILD_EXEC         := yes
# Sets INSTALLABLE_DIRS
include            $(MOOSE_DIR)/modules/combined/combined_installable_dirs.mk
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here
