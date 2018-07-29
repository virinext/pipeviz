# Location of our own features:
command = $$[QT_INSTALL_BINS]/qmake -set QMAKEFEATURES $$_PRO_FILE_PWD_
system($$command)|error("Failed to run: $$command")

include(pipeviz.pri)
