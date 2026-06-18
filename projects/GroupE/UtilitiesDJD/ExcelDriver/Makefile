# Makefile for ExcelDriver test programs
#
# Requires: MSVC (cl.exe) — run from a Visual Studio Developer Command Prompt
#           x64 Native Tools Command Prompt for VS 2022 (or any VS version)
#
# Usage:
#   nmake all          - build all three test programs
#   nmake run_single   - build and run TestSingleCurve
#   nmake run_two      - build and run TestTwoCurve
#   nmake run_multi    - build and run TestMultiCurve
#   nmake run          - build and run all tests sequentially
#   nmake setup_boost  - download Boost headers to C:\local\boost_1_87_0
#   nmake clean        - remove build artifacts (.exe, .obj, .tlh, .tli)
#
# Optional: override Boost include directory if not in INCLUDE already
#   nmake BOOST_INC="C:\boost_1_84_0" all

CXX      = cl
CXXFLAGS = /EHsc /std:c++17 /I"..\.."

BOOST_VERSION = 1_87_0
BOOST_DEFAULT = C:\local\boost_$(BOOST_VERSION)

!IFNDEF BOOST_INC
!IF EXIST("$(BOOST_DEFAULT)\boost\lexical_cast.hpp")
BOOST_INC = $(BOOST_DEFAULT)
!ENDIF
!ENDIF

!IFDEF BOOST_INC
CXXFLAGS = $(CXXFLAGS) /I"$(BOOST_INC)"
!ELSE
!MESSAGE ==============================================================
!MESSAGE Boost headers not found.
!MESSAGE Run: nmake setup_boost
!MESSAGE Or : nmake BOOST_INC="C:\path\to\boost" all
!MESSAGE ==============================================================
!ENDIF

COMMON_SRCS = ExcelDriverLite.cpp \
              ..\BitsAndPieces\StringConversions.cpp \
              ..\ExceptionClasses\DatasimException.cpp

# ── Targets ────────────────────────────────────────────────────────────────────

all: TestSingleCurve.exe TestTwoCurve.exe TestMultiCurve.exe

TestSingleCurve.exe: TestSingleCurve.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) TestSingleCurve.cpp $(COMMON_SRCS) /Fe:TestSingleCurve.exe

TestTwoCurve.exe: TestTwoCurve.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) TestTwoCurve.cpp $(COMMON_SRCS) /Fe:TestTwoCurve.exe

TestMultiCurve.exe: TestMultiCurve.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) TestMultiCurve.cpp $(COMMON_SRCS) /Fe:TestMultiCurve.exe

# ── Run targets ────────────────────────────────────────────────────────────────

run_single: TestSingleCurve.exe
	TestSingleCurve.exe

run_two: TestTwoCurve.exe
	TestTwoCurve.exe

run_multi: TestMultiCurve.exe
	TestMultiCurve.exe

run: TestSingleCurve.exe TestTwoCurve.exe TestMultiCurve.exe
	TestSingleCurve.exe
	TestTwoCurve.exe
	TestMultiCurve.exe

setup_boost:
	powershell -NoProfile -ExecutionPolicy Bypass -Command "$$ErrorActionPreference='Stop'; $$v='$(BOOST_VERSION)'; $$dir='C:\\local\\boost_'+$$v; $$zip='C:\\local\\boost_'+$$v+'.zip'; if (!(Test-Path 'C:\\local')) { New-Item -ItemType Directory -Path 'C:\\local' | Out-Null }; if (!(Test-Path ($$dir+'\\boost\\lexical_cast.hpp'))) { Invoke-WebRequest -Uri ('https://archives.boost.io/release/'+($$v -replace '_','.')+'/source/boost_'+$$v+'.zip') -OutFile $$zip; Expand-Archive -Path $$zip -DestinationPath 'C:\\local' -Force }; if (Test-Path ($$dir+'\\boost\\lexical_cast.hpp')) { Write-Host 'Boost ready at' $$dir } else { throw 'Boost installation failed' }"

# ── Clean ──────────────────────────────────────────────────────────────────────

clean:
	-del /Q *.exe *.obj *.tlh *.tli 2>nul
