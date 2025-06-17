project('wallet-sync-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20'])

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

spdlog_dep = dependency('spdlog')
json_dep   = dependency('nlohmann_json', required: false)

# ───── Source Files ─────
srcs = files(
  'src/main.cpp',
  'src/redis_listener.cpp',
  'src/wallet_store.cpp',
  'src/health_checker.cpp',
  'src/config_loader.cpp'
)

# ───── Include Paths ─────
inc = include_directories('include', '.')

# ───── Executable ─────
executable('wallet-sync-service',
  sources: srcs,
  include_directories: inc,
  dependencies: [
    boost_dep,
    spdlog_dep,
    redis_dep,
    json_dep
  ]
)
