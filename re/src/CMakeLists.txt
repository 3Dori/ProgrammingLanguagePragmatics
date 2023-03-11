include_directories(${PROJECT_SOURCE_DIR}/RE/inc/)
add_library(
    RE
    FA.cc
    RE.cc
    NodeManager.cc
    DFAMinimizer.cc
)

# include-what-you-use
find_program(iwyu_path NAMES include-what-you-use iwyu REQUIRED)
# set_property(TARGET RE PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})