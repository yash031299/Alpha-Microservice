project('kline-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20'])

add_project_arguments('-w', language: 'cpp')  # Suppress all warnings

# Compiler helper
cc = meson.get_compiler('cpp')

# ──────────────── Dependencies ────────────────

# Boost (from Homebrew path)
boost_inc = include_directories('/opt/homebrew/opt/boost/include')
boost_dep = declare_dependency(
  include_directories: boost_inc,
  link_args: ['-L/opt/homebrew/opt/boost/lib', '-lboost_system', '-lboost_thread']
)

# Spdlog (via pkg-config or Homebrew)
spdlog_dep = dependency('spdlog', required: true)

# Hiredis (manual link if not available via pkg-config)
hiredis_dep = declare_dependency(
  include_directories: include_directories('/opt/homebrew/include'),
  link_args: ['-L/opt/homebrew/lib', '-lhiredis']
)

# ──────────────── Includes ────────────────
inc = include_directories('include', '.')

# ──────────────── Source Files ────────────────
srcs = files(
  'src/main.cpp',
  'src/candle_aggregator.cpp',
  'src/kline_scheduler.cpp',
  'src/health_check.cpp',
  'src/config_loader.cpp'
)

# ──────────────── Executable Target ────────────────
executable('kline-service',
  sources: srcs,
  include_directories: inc,
  dependencies: [
    boost_dep,
    spdlog_dep,
    hiredis_dep
  ]
)
