project('pnl-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20']
)

add_project_arguments('-w', language: 'cpp')  # Disable all warnings

cc = meson.get_compiler('cpp')

# ───── Dependencies ─────
boost_inc = include_directories('/opt/homebrew/opt/boost/include')
boost_dep = declare_dependency(
  include_directories: boost_inc,
  link_args: ['-L/opt/homebrew/opt/boost/lib', '-lboost_system', '-lboost_thread']
)

redis_inc = include_directories('/opt/homebrew/include', '/opt/homebrew/include/sw')
redis_dep = declare_dependency(
  include_directories: redis_inc,
  link_args: ['-L/opt/homebrew/lib', '-lredis++', '-lhiredis']
)

spdlog_dep       = dependency('spdlog')
grpc_dep         = dependency('grpc++', required: true)
protobuf_dep     = dependency('protobuf', required: true)
nlohmann_json_dep = dependency('nlohmann_json')
doctest_dep      = dependency('doctest', required: true, fallback: ['doctest', 'doctest_dep'])

all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  grpc_dep,
  protobuf_dep,
  nlohmann_json_dep
]

inc = include_directories('include')

# ───── Source Files ─────
sources = files(
  'src/main.cpp',
  'src/grpc_interface.cpp',
  'src/pnl_engine.cpp',
  'src/equity_calculator.cpp',
  'src/realized_pnl_store.cpp',
  'src/symbol_client.cpp',
  'src/health_check.cpp',
  'src/config_loader.cpp',
  'src/pnl.pb.cc',
  'src/pnl.grpc.pb.cc',
  'src/symbol.pb.cc',
  'src/symbol.grpc.pb.cc'
)

# ───── Executable ─────
executable('pnl-service',
  sources,
  include_directories: inc,
  dependencies: all_deps
)

# ───── Unit Tests ─────
test_sources = files(
  'tests/test_pnl_engine.cpp',
  'tests/test_equity_calculator.cpp'
)

shared_sources = files(
  'src/grpc_interface.cpp',
  'src/pnl_engine.cpp',
  'src/equity_calculator.cpp',
  'src/realized_pnl_store.cpp',
  'src/symbol_client.cpp',
  'src/health_check.cpp',
  'src/config_loader.cpp',
  'src/pnl.pb.cc',
  'src/pnl.grpc.pb.cc',
  'src/symbol.pb.cc',
  'src/symbol.grpc.pb.cc'
)

test_exe = executable('test-pnl-service',
  test_sources + shared_sources,
  include_directories: inc,
  dependencies: all_deps + [doctest_dep]
)

test('pnl-unit-tests', test_exe)
