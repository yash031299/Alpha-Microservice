project('order-router-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20'])

add_project_arguments('-w', language: 'cpp')  # Disable all warnings

# ───── Dependencies ─────

# Boost (system/thread)
boost_inc = include_directories('/opt/homebrew/opt/boost/include')
boost_dep = declare_dependency(
  include_directories: boost_inc,
  link_args: ['-L/opt/homebrew/opt/boost/lib', '-lboost_system', '-lboost_thread']
)

# spdlog (logging)
spdlog_dep = dependency('spdlog', required: true)

# hiredis (Redis C library)
hiredis_dep = declare_dependency(
  include_directories: include_directories('/opt/homebrew/include'),
  link_args: ['-L/opt/homebrew/lib', '-lhiredis']
)

# grpc / grpc++
grpc_dep     = dependency('grpc', required: true)
grpcpp_dep   = dependency('grpc++', required: true)

# protobuf (used by grpc)
protobuf_dep = dependency('protobuf', required: true)

# rapidjson (header-only)
rapidjson_dep = declare_dependency(
  include_directories: include_directories('/opt/homebrew/include')
)

# nlohmann_json (optional)
json_dep = dependency('nlohmann_json', required: false)

# ───── Source Files ─────
srcs = [
  'main.cpp',
  'src/config_loader.cpp',
  'src/grpc_client.cpp',
  'src/order_router.cpp',
  'src/match_engine_client.cpp',
  'src/rms_client.cpp',
  'src/order.pb.cc',
  'src/order.grpc.pb.cc',
  'src/match.pb.cc',
  'src/match.grpc.pb.cc',
  'src/pnl.pb.cc',
  'src/pnl.grpc.pb.cc'
]

# ───── Include Paths ─────
inc = include_directories('include', '.')

# ───── Final Executable ─────
executable('order-router-service',
  srcs,
  include_directories: inc,
  dependencies: [
    boost_dep,
    spdlog_dep,
    hiredis_dep,
    grpc_dep,
    grpcpp_dep,
    protobuf_dep,
    rapidjson_dep,
    json_dep
  ]
)
