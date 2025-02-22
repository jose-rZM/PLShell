project('plshell', 'cpp', version: '1.0.0', default_options: ['cpp_std=c++20', 'buildtype=debug'])

boost_dep = dependency('boost', modules: ['program_options'], static: true)

gtest_dep = dependency('gtest', required: false)

executable('plshell',
    files(
        'src/main.cpp',
        'src/shell/shell.cpp',
        'src/shell/commands.cpp',
        'src/parser/ll1_parser.cpp',
        'src/parser/slr1_parser.cpp',
        'src/parser/grammar.cpp',
        'src/parser/lr0_item.cpp',
        'src/parser/symbol_table.cpp',
        'src/parser/grammar.cpp'
    ),
    dependencies: [boost_dep]
)

# Tests
test_exe = executable('tests',
    files(
        'tests/tests.cpp',
        'src/parser/ll1_parser.cpp',
        'src/parser/slr1_parser.cpp',
        'src/parser/grammar.cpp',
        'src/parser/lr0_item.cpp',
        'src/parser/symbol_table.cpp',
        'src/parser/grammar.cpp'
    ),
    dependencies: [gtest_dep],
    link_args: ['-lpthread']
)
test('Tests', test_exe)

custom_target(
    'run_tests',
    output: 'test-stamp',
    command: [test_exe.full_path()],
    depends: test_exe,
    build_by_default: false
)