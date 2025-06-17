project('user-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20']
)

# Disable all warnings globally
add_project_arguments('-w', language: 'cpp')

# Dependencies
boost_dep = dependency('boost', modules: ['system'], required: false)
redis_dep = dependency('hiredis', required: true)
spdlog_dep = dependency('spdlog', required: true)
grpc_dep = dependency('grpc++', required: true)
protobuf_dep = dependency('protobuf', required: true)

inc = include_directories('include', 'src')

srcs = [
  'src/main.cpp',
  'src/redis_safe_client.cpp',
  'src/user_wallet_store.cpp',
  'src/user_metadata_store.cpp',
  'src/margin_listener.cpp',
  'src/scheduler.cpp',
  'src/health_checker.cpp',
  'src/user.pb.cc',
  'src/user.grpc.pb.cc',
  'src/grpc_user_server.cpp' 
]

executable('user-service',
  srcs,
  include_directories: inc,
  dependencies: [boost_dep, redis_dep, spdlog_dep, grpc_dep, protobuf_dep]
)
