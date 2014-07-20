include (../../shared.pri)


HEADERS       = edit_phov_factory.h \
                edit_phov.h
				 
SOURCES       = edit_phov_factory.cpp \
                edit_phov.cpp

TARGET        = edit_phov

RESOURCES     = edit_phov.qrc

QT += network
