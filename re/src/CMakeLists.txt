include_directories(${PROJECT_SOURCE_DIR}/RE/inc/)
add_library(
    RE
    FA.cc
    RE.cc
    StateManager.cc
    DFAMinimizer.cc
)
