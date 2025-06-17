project('match-engine', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20'])

add_project_arguments('-w', language: 'cpp')  # Disable warnings

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

spdlog_dep = dependency('spdlog')
json_dep   = dependency('nlohmann_json')
grpc_dep   = dependency('grpc++', required: true)
protobuf_dep = dependency('protobuf', required: true)

# Group all dependencies for reuse
all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  json_dep,
  grpc_dep,
  protobuf_dep
]

# ───── Source Files ─────
srcs = files(
  'src/main.cpp',
  'src/book.cpp',
  'src/matcher.cpp',
  'src/trade_publisher.cpp',
  'src/env_loader.cpp',
  'src/redis_listener.cpp',
  'src/order_parser.cpp',
  'src/grpc_server.cpp',
  'src/health_check.cpp',
  'src/metrics.cpp',
  'src/match.pb.cc',
  'src/match.grpc.pb.cc'
)

# ───── Include Paths ─────
inc = include_directories('include')

# ───── Main Executable ─────
executable('match-engine',
  sources: srcs,
  include_directories: inc,
  dependencies: all_deps
)

# ───── Unit Test Executable ─────
test_exe = executable('test-orderbook',
  sources: ['tests/test_orderbook.cpp', 'src/book.cpp'],  # ← include book.cpp here
  include_directories: inc,
  dependencies: all_deps
)

test('OrderBook Unit Tests', test_exe)
