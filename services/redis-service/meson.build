project('redis-service', 'cpp',
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

spdlog_dep = dependency('spdlog', required: true)
nlohmann_json_dep = dependency('nlohmann_json', required: true)

all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  nlohmann_json_dep
]

inc = include_directories('include')

# ───── Source Files ─────
sources = files(
  'src/main.cpp',
  'src/redis_connection.cpp',
  'src/order_updater.cpp',
  'src/trade_broadcaster.cpp',
  'src/redis_pool.cpp',
  'src/redis_pool_singleton.cpp',
  'src/health_check.cpp'
)

# ───── Executable ─────
executable('redis-service',
  sources,
  include_directories: inc,
  dependencies: all_deps
)
