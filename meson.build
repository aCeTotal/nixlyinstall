project('nixlyinstall', 'cpp',
  version: '0.1',
  default_options: ['cpp_std=c++20']
)

# Install desktop file
install_data('nixlyinstall.desktop',
  install_dir: join_paths(get_option('datadir'), 'applications')
)

subdir('src')
