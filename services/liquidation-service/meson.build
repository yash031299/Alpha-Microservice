project('liquidation-service', 'cpp',
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

spdlog_dep   = dependency('spdlog')
json_dep     = dependency('nlohmann_json')
prometheus_dep = declare_dependency(
  include_directories: include_directories('/usr/local/include'),
  link_args: ['-L/usr/local/lib', '-lprometheus-cpp-core', '-lprometheus-cpp-pull']
)

httplib_dep = declare_dependency(
  include_directories: include_directories('/opt/homebrew/include')
)

# ───── Source Files ─────
srcs = files(
  'src/main.cpp',
  'src/config_loader.cpp',
  'src/default_strategy.cpp',
  'src/http_health_server.cpp',
  'src/liquidation_controller.cpp',
  'src/redis_connection_pool.cpp',
  'src/redis_user_client.cpp',
  'src/utils/metrics.cpp',
  'src/utils/retry_utils.cpp'
)

# ───── Include Paths ─────
inc = include_directories('include', 'include/core', 'include/utils', 'include/interfaces')

# ───── Executable ─────
executable('liquidation-service',
  sources: srcs,
  include_directories: inc,
  dependencies: [
    boost_dep,
    spdlog_dep,
    redis_dep,
    json_dep,
    prometheus_dep,
    httplib_dep
  ]
)
