CXX      := g++
RC       := windres
COMMON_CFLAGS := -mavx2 -march=native

CPP_SOURCES := \
    SplitterE.cpp \
    Soma.cpp \
    EView.cpp \
    LView.cpp \
    FrameProc.cpp \
    ExtraTree.cpp \
    ExtraFrame.cpp

RC_SOURCES := Soma.rc

BUILDDIR := build
OBJDIR_RELEASE := $(BUILDDIR)/Release/obj
OBJDIR_DEBUG   := $(BUILDDIR)/Debug/obj
BINDIR_RELEASE := $(BUILDDIR)/Release
BINDIR_DEBUG   := $(BUILDDIR)/Debug

OBJS_RELEASE := $(addprefix $(OBJDIR_RELEASE)/,$(CPP_SOURCES:.cpp=.o)) \
                $(addprefix $(OBJDIR_RELEASE)/,$(RC_SOURCES:.rc=_rc.o))

OBJS_DEBUG   := $(addprefix $(OBJDIR_DEBUG)/,$(CPP_SOURCES:.cpp=.o)) \
                $(addprefix $(OBJDIR_DEBUG)/,$(RC_SOURCES:.rc=_rc.o))

# --- Release ---
CFLAGS_RELEASE := $(COMMON_CFLAGS) -O3 -Wall -fexceptions -Wexpansion-to-defined -Wcast-function-type -DNDEBUG -D_WINDOWS -D_MBCS
LDFLAGS_RELEASE := -m64 -mwindows -s
LIBS_RELEASE := -lkernel32 -luser32 -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32 -lpthread

# --- Debug ---
CFLAGS_DEBUG := $(COMMON_CFLAGS) -Wall -fexceptions -g -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS
LDFLAGS_DEBUG := -m64 -mwindows
LIBS_DEBUG := -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -lcomctl32 -lpthread

# --- Targets ---
all: release debug clean

release: $(OBJS_RELEASE)
	$(CXX) $(LDFLAGS_RELEASE) -o $(BINDIR_RELEASE)/Soma $(OBJS_RELEASE) $(LIBS_RELEASE)

debug: $(OBJS_DEBUG)
	$(CXX) $(LDFLAGS_DEBUG) -o $(BINDIR_DEBUG)/dSoma $(OBJS_DEBUG) $(LIBS_DEBUG)

# --- rules Release ---
$(OBJDIR_RELEASE)/%.o: %.cpp
	@mkdir -p $(OBJDIR_RELEASE)
	$(CXX) $(CFLAGS_RELEASE) -c $< -o $@

$(OBJDIR_RELEASE)/%_rc.o: %.rc
	@mkdir -p $(OBJDIR_RELEASE)
	$(RC) -i $< -o $@

# --- rules Debug ---
$(OBJDIR_DEBUG)/%.o: %.cpp
	@mkdir -p $(OBJDIR_DEBUG)
	$(CXX) $(CFLAGS_DEBUG) -c $< -o $@

$(OBJDIR_DEBUG)/%_rc.o: %.rc
	@mkdir -p $(OBJDIR_DEBUG)
	$(RC) -i $< -o $@

clean:
	rm -rf $(OBJDIR_RELEASE)
	rm -rf $(OBJDIR_DEBUG)