project('order-router-service', 'cpp',
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
json_dep   = dependency('nlohmann_json')
grpc_dep   = dependency('grpc++', required: true)
protobuf_dep = dependency('protobuf', required: true)

all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  json_dep,
  grpc_dep,
  protobuf_dep
]

srcs = files(
  'main.cpp',
  'src/order_router.cpp',
  'src/grpc_client.cpp',
  'src/order.pb.cc',
  'src/order.grpc.pb.cc'
)

inc = include_directories('include')

executable('order-router-service',
  sources: srcs,
  include_directories: inc,
  dependencies: all_deps
)
