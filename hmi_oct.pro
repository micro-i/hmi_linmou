TEMPLATE = subdirs

# main frame
SUBDIRS += main_frame

## plugin demo
SUBDIRS += oct_plugins

## octdevkit
#SUBDIRS += oct_devkit

## microdevkit
#SUBDIRS += microi_devkit

## micro-i statistic plugins
SUBDIRS += statistic_plugins


# camera acquisition
#SUBDIRS += camera_test

# adda card simluate prog
#SUBDIRS += adda_test

# translation stage
#SUBDIRS += translation_stage


# depends relation
#oct_plugins.depends = oct_devkit
main_frame.depends = statistic_plugins
main_frame.depends = oct_plugins
