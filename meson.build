project(
  'aoc2025',
  'c',
  default_options : ['c_std=gnu2x', 'c_args=-D_GNU_SOURCE', 'warning_level=2']
)

cc = meson.get_compiler('c')

clang_warnings = [
    # Implied by warning_level=2
    # '-Wall',  # reasonable and standard
    #'-Wextra',  # extra warnings
    '-Wshadow',  # warn the user if a variable declaration shadows one from a parent context
    '-Wcast-align',  # warn for potential performance problem casts
    '-Wunused',  # warn on anything being unused
    '-Wconversion',  # warn on type conversions that may lose data
    '-Wsign-conversion',  # warn on sign conversions
    '-Wnull-dereference',  # warn if a null dereference is detected
    '-Wdouble-promotion',  # warn if float is implicit promoted to double
    '-Wformat=2',  # warn on security issues around functions that format output (ie printf)
    '-Wno-unknown-pragmas',  # ignore IDE, GCC/CLANG specific pragmas
    '-Wimplicit-fallthrough',  # Warns when case statements fall-through.
]

gcc_warnings = clang_warnings + [
    '-Wmisleading-indentation',  # warn if indentation implies blocks where blocks do not exist
    '-Wduplicated-cond',  # warn if if / else chain has duplicated conditions
    '-Wduplicated-branches',  # warn if if / else branches have duplicated code
    '-Wlogical-op',  # warn about logical operations being used where bitwise were probably wanted
    '-Wno-maybe-uninitialized',  # false positives if asan is enabled: https://gcc.gnu.org/bugzilla//show_bug.cgi?id=1056h6
]

if cc.get_id() == 'msvc'
    add_project_arguments(msvc_warnings, language : 'c')
elif cc.get_id() == 'clang'
    add_project_arguments(clang_warnings, language : 'c')
elif cc.get_id() == 'gcc'
    add_project_arguments(gcc_warnings, language : 'c')
endif

day1 = executable('day1', 'src/day1.c')
test('day1', day1)
day2 = executable('day2', 'src/day2.c')
test('day2', day2)
day3 = executable('day3', 'src/day3.c')
test('day3', day3)
day4 = executable('day4', 'src/day4.c')
test('day4', day4)
day5 = executable('day5', 'src/day5.c')
test('day5', day5)
day6 = executable('day6', 'src/day6.c')
test('day6', day6)
day7 = executable('day7', 'src/day7.c')
test('day7', day7)
day8 = executable('day8', 'src/day8.c')
test('day8', day8)
day9 = executable('day9', 'src/day9.c')
test('day9', day9)
