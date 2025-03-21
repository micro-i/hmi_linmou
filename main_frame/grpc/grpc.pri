DISTFILES += \
    $$PWD/algorithm.proto

HEADERS += \
    $$PWD/algorithm.grpc.pb.h \
    $$PWD/algorithm.pb.h

SOURCES += \
    $$PWD/algorithm.grpc.pb.cc \
    $$PWD/algorithm.pb.cc


GRPC= $$shell_path(C:/libraries/grpc/)
INCLUDEPATH += $$shell_path($$GRPC/include)
CONFIG(debug, debug|release) {
        unix{
        }
        win32{
                LIBS += $$GRPC/lib/debug/*.lib
                LIBS += -LC:/Libraries
        }
}

CONFIG(release, debug|release) {
        unix{
        }
        win32{
                LIBS += $$GRPC/lib/release/*.lib
                LIBS += -LC:/Libraries
        }
}
