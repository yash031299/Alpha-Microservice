project('order-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20']
)

add_project_arguments('-w', language: 'cpp')

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

spdlog_dep     = dependency('spdlog')
grpc_dep       = dependency('grpc++', required: true)
protobuf_dep   = dependency('protobuf', required: true)
prometheus_core_dep = dependency('prometheus-cpp', modules: ['prometheus-cpp::core'], required: true)
prometheus_pull_dep = dependency('prometheus-cpp', modules: ['prometheus-cpp::pull'], required: true)
doctest_dep = dependency('doctest', required: true, fallback: ['doctest', 'doctest_dep'])

all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  grpc_dep,
  protobuf_dep,
  prometheus_core_dep,
  prometheus_pull_dep
]

inc = include_directories('include')

# ───── Source Files ─────
sources = files(
  'src/metrics.cpp',
  'src/main.cpp',
  'src/controller.cpp',
  'src/grpc_client.cpp',
  'src/symbol_client.cpp',
  'src/user_client.cpp',
  'src/validator.cpp',
  'src/redis_publisher.cpp',
  'src/order.pb.cc',
  'src/order.grpc.pb.cc',
  'src/user.pb.cc',
  'src/user.grpc.pb.cc',
  'src/symbol.pb.cc',
  'src/symbol.grpc.pb.cc'
)

# ───── Shared Source Files (without main.cpp) ─────
shared_sources = files(
  'src/metrics.cpp',
  'src/controller.cpp',
  'src/grpc_client.cpp',
  'src/symbol_client.cpp',
  'src/user_client.cpp',
  'src/validator.cpp',
  'src/redis_publisher.cpp',
  'src/order.pb.cc',
  'src/order.grpc.pb.cc',
  'src/user.pb.cc',
  'src/user.grpc.pb.cc',
  'src/symbol.pb.cc',
  'src/symbol.grpc.pb.cc'
)

# ───── Main Executable ─────
executable('order-service',
  sources,
  include_directories: inc,
  dependencies: all_deps
)

# ───── Test Executable ─────
test_sources = files(
  'tests/test_main.cpp',
  'tests/test_validator.cpp',
  'tests/test_execution_client.cpp',
  'tests/test_controller.cpp'
)

test_exe = executable('test-order-service',
  test_sources + shared_sources,
  include_directories: inc,
  dependencies: all_deps + [doctest_dep]
)

test('order-service-tests', test_exe)
